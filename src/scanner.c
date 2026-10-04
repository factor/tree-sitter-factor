#include "tree_sitter/parser.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <wctype.h>

#include "reserved.h"

enum TokenType {
  WORD, NAME, RAW_NAME, EFFECT_NAME, FFI_NAME, FUNCTOR_NAME, LIST_NAME, NUMBER, STRING, RAW_STRING, REGEXP,
  COMMENT, SHEBANG, CHARACTER, COLLECTION_START, EFFECT_START, QUOTATION_START, STRING_BODY, ERROR_SENTINEL,
};

typedef struct {
  char *text;
  size_t capacity;
} Scanner;

static bool space(int32_t c) { return c == 0xfeff || iswspace((wint_t)c); }
static bool boundary(TSLexer *lexer) { return lexer->eof(lexer) || space(lexer->lookahead); }
static void advance(TSLexer *lexer) { lexer->advance(lexer, false); }

static bool append(Scanner *scanner, size_t *length, int32_t c) {
  if (*length + 2 > scanner->capacity) {
    size_t capacity = scanner->capacity ? scanner->capacity * 2 : 128;
    char *text = realloc(scanner->text, capacity);
    if (!text) return false;
    scanner->text = text;
    scanner->capacity = capacity;
  }
  // Only ASCII text is needed for lexical classification. Unicode words are
  // preserved in the tree directly from the input, not from this scratch buffer.
  scanner->text[(*length)++] = c < 128 ? (char)c : '\x7f';
  scanner->text[*length] = '\0';
  return true;
}

static bool reserved(const char *text) {
  size_t left = 0, right = sizeof(RESERVED_WORDS) / sizeof(RESERVED_WORDS[0]);
  while (left < right) {
    size_t middle = left + (right - left) / 2;
    int order = strcmp(text, RESERVED_WORDS[middle]);
    if (!order) return true;
    if (order < 0) right = middle; else left = middle + 1;
  }
  return false;
}

static bool delimiter(const char *s) {
  static const char *const delimiters[] = {
    ";", "[", "]", "{", "}", "(", ")", "|", "--", "<", "=>", "<<", ">>",
    "<PRIVATE", "PRIVATE>", "read-only", "initial:", "...", ",",
  };
  for (size_t i = 0; i < sizeof(delimiters) / sizeof(delimiters[0]); i++)
    if (!strcmp(s, delimiters[i])) return true;
  return false;
}

static int digit(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return -1;
}

static bool digits(const char **s, int radix) {
  const char *p = *s;
  bool any = false, separated = false;
  while (*p) {
    if (digit(*p) >= 0 && digit(*p) < radix) {
      any = true; separated = false; p++;
    } else if (any && !separated && (*p == ',' || *p == '_')) {
      separated = true; p++;
    } else break;
  }
  *s = p;
  return any && !separated;
}

// Recognize Factor's whole-token numbers; do not split 1array or 2dup.
static bool real_number(const char *s) {
  bool negative = *s == '-';
  if (*s == '+' || *s == '-') s++;
  int radix = 10;
  if (s[0] == '0' && s[1]) {
    if (s[1] == 'b' || s[1] == 'B') radix = 2;
    if (s[1] == 'o' || s[1] == 'O') radix = 8;
    if (s[1] == 'x' || s[1] == 'X') radix = 16;
    if (radix != 10) s += 2;
  }
  bool integer = digits(&s, radix);
  if (*s == '/') {
    if (!integer) return false;
    s++;
    const char *denominator = s;
    if (!digits(&s, radix)) return false;
    bool nonzero = false;
    for (const char *p = denominator; p < s; p++) if (digit(*p) > 0) nonzero = true;
    bool floating = *s == '.';
    if (*s == '.') s++; // Floating ratios, including 1/0. and 0/0.
    return !*s && (nonzero || floating);
  }
  if (*s == (negative ? '-' : '+')) {
    if (!integer) return false;
    s++;
    if (!digits(&s, radix) || *s++ != '/') return false;
    return digits(&s, radix) && !*s;
  }
  bool point = false;
  if (*s == '.') {
    s++; point = true;
    if (*s && *s != 'e' && *s != 'E' && *s != 'p' && *s != 'P') {
      if (!digits(&s, radix)) return false;
    } else if (!integer) return false;
  } else if (!integer) return false;
  bool exponent = (radix == 10 && (*s == 'e' || *s == 'E')) ||
                  (radix != 10 && (*s == 'p' || *s == 'P'));
  if (exponent) {
    s++;
    if (*s == '+' || *s == '-') s++;
    if (!digits(&s, 10)) return false;
  }
  return !*s && (radix == 10 || !point || exponent);
}

static bool number(char *s) {
  if (real_number(s)) return true;
  size_t length = strlen(s);
  if (length < 2 || s[length - 1] != 'j') return false;
  s[length - 1] = '\0';
  bool result = real_number(s) || !strcmp(s, "+") || !strcmp(s, "-");
  for (size_t i = 1; !result && i < length - 1; i++) {
    if (s[i] != '+' && s[i] != '-') continue;
    char sign = s[i]; s[i] = '\0';
    bool real = real_number(s); s[i] = sign;
    result = real && (real_number(s + i) || !strcmp(s + i, "+") || !strcmp(s + i, "-"));
  }
  s[length - 1] = 'j';
  return result;
}

static bool quoted_string(TSLexer *lexer) {
  advance(lexer); // Opening quote.
  bool triple = false;
  if (lexer->lookahead == '"') {
    advance(lexer);
    if (lexer->lookahead != '"') return boundary(lexer);
    advance(lexer); triple = true;
  }
  unsigned quotes = 0;
  while (!lexer->eof(lexer)) {
    int32_t c = lexer->lookahead;
    advance(lexer);
    if (c == '\\') {
      if (lexer->eof(lexer)) return false;
      advance(lexer); quotes = 0;
    } else if (c == '"') {
      if (!triple || ++quotes == 3) return boundary(lexer);
    } else quotes = 0;
  }
  return false;
}

static bool until_pair(TSLexer *lexer, int32_t first, int32_t second) {
  bool seen = false;
  while (!lexer->eof(lexer)) {
    int32_t c = lexer->lookahead;
    advance(lexer);
    if (seen && c == second) return boundary(lexer);
    seen = c == first;
  }
  return false;
}

static bool bracket_string(TSLexer *lexer, size_t equals) {
  while (!lexer->eof(lexer)) {
    if (lexer->lookahead != ']') { advance(lexer); continue; }
    advance(lexer);
    size_t count = 0;
    while (lexer->lookahead == '=') { count++; advance(lexer); }
    if (count == equals && lexer->lookahead == ']') {
      advance(lexer);
      return boundary(lexer);
    }
    // Retain a candidate ']' so overlapping closing delimiters work.
  }
  return false;
}

static bool until_text(TSLexer *lexer, const char *end) {
  size_t matched = 0, length = strlen(end);
  while (!lexer->eof(lexer)) {
    int32_t c = lexer->lookahead; advance(lexer);
    if (c == end[matched]) matched++;
    else matched = c == end[0] ? 1 : 0;
    if (matched == length) return boundary(lexer);
  }
  return false;
}

void *tree_sitter_factor_external_scanner_create(void) {
  return calloc(1, sizeof(Scanner));
}
void tree_sitter_factor_external_scanner_destroy(void *payload) {
  Scanner *scanner = payload;
  free(scanner->text); free(scanner);
}
unsigned tree_sitter_factor_external_scanner_serialize(void *payload, char *buffer) {
  (void)payload; (void)buffer; return 0;
}
void tree_sitter_factor_external_scanner_deserialize(void *payload, const char *buffer, unsigned length) {
  (void)payload; (void)buffer; (void)length;
}

bool tree_sitter_factor_external_scanner_scan(void *payload, TSLexer *lexer, const bool *valid) {
  Scanner *scanner = payload;
  if (valid[ERROR_SENTINEL]) return false;
  if (valid[STRING_BODY]) {
    while (lexer->lookahead == ' ' || lexer->lookahead == '\t' || lexer->lookahead == '\r')
      lexer->advance(lexer, true);
    if (lexer->lookahead != '\n') return false;
    advance(lexer);
    while (!lexer->eof(lexer)) {
      if (lexer->get_column(lexer) == 0 && lexer->lookahead == ';') {
        lexer->mark_end(lexer); advance(lexer);
        if (lexer->eof(lexer) || lexer->lookahead == '\n' || lexer->lookahead == '\r') {
          lexer->result_symbol = STRING_BODY; return true;
        }
      } else advance(lexer);
    }
    return false;
  }
  while (space(lexer->lookahead)) lexer->advance(lexer, true);
  if (lexer->eof(lexer)) return false;

  if (valid[CHARACTER]) {
    while (!boundary(lexer)) advance(lexer);
    lexer->result_symbol = CHARACTER;
    return true;
  }

  if (lexer->lookahead == '"') {
    if (valid[RAW_NAME]) {
      advance(lexer); lexer->result_symbol = RAW_NAME; return true;
    }
    if (!valid[STRING] || !quoted_string(lexer)) return false;
    lexer->mark_end(lexer); lexer->result_symbol = STRING; return true;
  }

  size_t length = 0;
  while (!boundary(lexer)) {
    if (lexer->lookahead == '"' && !valid[RAW_NAME] && !valid[NAME] && !valid[EFFECT_NAME]) {
      if (!valid[STRING] || !quoted_string(lexer)) return false;
      lexer->mark_end(lexer); lexer->result_symbol = STRING; return true;
    }
    if (!append(scanner, &length, lexer->lookahead)) return false;
    bool suffix = (valid[EFFECT_NAME] && lexer->lookahead == ':') ||
                  (valid[FFI_NAME] && lexer->lookahead == ',');
    advance(lexer);
    if (!suffix) lexer->mark_end(lexer);
  }
  const char *text = scanner->text;

  if (valid[SHEBANG] && !strncmp(text, "#!", 2)) {
    while (!lexer->eof(lexer) && lexer->lookahead != '\n') advance(lexer);
    lexer->mark_end(lexer); lexer->result_symbol = SHEBANG; return true;
  }
  if (valid[COMMENT] && !strcmp(text, "!")) {
    while (!lexer->eof(lexer) && lexer->lookahead != '\n') advance(lexer);
    lexer->mark_end(lexer); lexer->result_symbol = COMMENT; return true;
  }
  if (valid[RAW_NAME]) {
    lexer->mark_end(lexer); lexer->result_symbol = RAW_NAME; return true;
  }
  if (valid[LIST_NAME] && strcmp(text, ";")) {
    lexer->mark_end(lexer); lexer->result_symbol = LIST_NAME; return true;
  }
  if (valid[COMMENT] && (!strcmp(text, "/*") || !strcmp(text, "(("))) {
    bool block = text[0] == '/';
    if (!until_pair(lexer, block ? '*' : ')', block ? '/' : ')')) return false;
    lexer->mark_end(lexer); lexer->result_symbol = COMMENT; return true;
  }

  if (valid[RAW_STRING] && (!strcmp(text, "[I") || !strcmp(text, "[XML") || !strcmp(text, "<XML"))) {
    const char *end = !strcmp(text, "[I") ? "I]" : (!strcmp(text, "[XML") ? "XML]" : "XML>");
    if (!until_text(lexer, end)) return false;
    lexer->mark_end(lexer); lexer->result_symbol = RAW_STRING; return true;
  }

  // Standard and vocabulary-prefixed [=*[ strings share matching delimiters.
  const char *open = strchr(text, '[');
  if (open && open[1] != '|' && open[1] != 'l') {
    const char *p = open + 1;
    while (*p == '=') p++;
    if (*p == '[' && !p[1]) {
      bool comment = text[0] == '!';
      if (!(comment ? valid[COMMENT] : valid[RAW_STRING])) return false;
      size_t equals = (size_t)(p - open - 1);
      if (!bracket_string(lexer, equals)) return false;
      lexer->mark_end(lexer); lexer->result_symbol = comment ? COMMENT : RAW_STRING;
      return true;
    }
  }

  if (valid[FUNCTOR_NAME] && strcmp(text, "WHERE")) {
    lexer->mark_end(lexer); lexer->result_symbol = FUNCTOR_NAME; return true;
  }
  if (valid[FFI_NAME] && !delimiter(text)) {
    if (text[length - 1] != ',') lexer->mark_end(lexer);
    lexer->result_symbol = FFI_NAME; return true;
  }
  if (valid[NAME] && !delimiter(text) && text[length - 1] != '{' &&
      (!reserved(text) || !strcmp(text, "t") || !strcmp(text, "f"))) {
    lexer->mark_end(lexer); lexer->result_symbol = NAME; return true;
  }
  if (valid[EFFECT_NAME] && text[0] != ':' &&
      (!delimiter(text) || !strcmp(text, "..."))) {
    // A trailing ':' is a separate type annotation delimiter in stack effects.
    if (text[length - 1] != ':') lexer->mark_end(lexer);
    lexer->result_symbol = EFFECT_NAME; return true;
  }

  if (valid[REGEXP] && !strcmp(text, "R/")) {
    while (space(lexer->lookahead)) advance(lexer);
    while (!lexer->eof(lexer)) {
      int32_t c = lexer->lookahead; advance(lexer);
      if (c == '\\') {
        if (lexer->eof(lexer)) return false;
        advance(lexer);
      } else if (c == '/') {
        while (!boundary(lexer)) advance(lexer);
        lexer->mark_end(lexer); lexer->result_symbol = REGEXP; return true;
      }
    }
    return false;
  }
  if (valid[COLLECTION_START] && text[length - 1] == '{') {
    lexer->mark_end(lexer); lexer->result_symbol = COLLECTION_START; return true;
  }
  if (valid[EFFECT_START] && length > 1 && text[length - 1] == '(') {
    lexer->mark_end(lexer); lexer->result_symbol = EFFECT_START; return true;
  }
  if (valid[QUOTATION_START] && length > 1 && text[length - 1] == '[') {
    lexer->mark_end(lexer); lexer->result_symbol = QUOTATION_START; return true;
  }
  if (number(scanner->text)) {
    if (!valid[NUMBER]) return false;
    lexer->mark_end(lexer); lexer->result_symbol = NUMBER; return true;
  }
  if (reserved(text) || text[0] == '"' || !strcmp(text, "R/")) return false;
  if (valid[WORD]) {
    lexer->mark_end(lexer); lexer->result_symbol = WORD; return true;
  }
  return false;
}
