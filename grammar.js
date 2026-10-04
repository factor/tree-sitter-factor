/**
 * Factor's parser is extensible. These rules model the standard syntax and
 * preserve arbitrary vocabulary words without trying to resolve imports.
 * The scanner enforces whole-word boundaries and reads delimited text.
 */
module.exports = grammar({
  name: 'factor',

  externals: $ => [
    $.word, $.name, $._raw_name, $._effect_name, $.number,
    $.string, $.raw_string, $.regexp, $.comment, $.shebang,
    $.character, $.collection_start, $._error_sentinel,
  ],

  extras: $ => [/\s+/, $.comment],

  rules: {
    source_file: $ => seq(optional($.shebang), repeat($._form)),

    _form: $ => choice(
      $.word_defn, $.method_defn, $.generic_defn, $.syntax_defn,
      $.import_defn, $.tuple_defn, $.class_defn, $.predicate_defn,
      $.constructor_defn, $.instance_defn, $.symbol_defn,
      $.constant_defn, $.alias_defn, $.deferred_defn, $.main_defn,
      $.private_section, $.compile_time, $._expression,
    ),

    word_defn: $ => seq(
      field('kind', choice(':', '::', 'MACRO:', 'MACRO::', 'MEMO:', 'MEMO::',
        'IDENTITY-MEMO:', 'IDENTITY-MEMO::', 'TYPED:', 'TYPED::')),
      field('name', alias($._raw_name, $.name)),
      field('effect', $.stack_effect),
      optional(field('body', $.expression)), ';',
    ),
    method_defn: $ => choice(
      seq('M:', field('class', $.name), field('name', alias($._raw_name, $.name)),
        optional(field('body', $.expression)), ';'),
      seq('M::', field('class', $.name), field('name', alias($._raw_name, $.name)),
        field('effect', $.stack_effect), optional(field('body', $.expression)), ';'),
    ),
    generic_defn: $ => choice(
      seq(field('kind', choice('GENERIC:', 'MATH:', 'PRIMITIVE:')),
        field('name', alias($._raw_name, $.name)), field('effect', $.stack_effect)),
      seq('GENERIC#:', field('name', alias($._raw_name, $.name)),
        field('dispatch', $.number), field('effect', $.stack_effect)),
      seq('HOOK:', field('name', alias($._raw_name, $.name)),
        field('variable', $.name), field('effect', $.stack_effect)),
    ),
    syntax_defn: $ => seq('SYNTAX:', field('name', alias($._raw_name, $.name)),
      optional(field('body', $.expression)), ';'),

    import_defn: $ => choice(
      seq('USING:', repeat(field('vocabulary', $.name)), ';'),
      seq(field('kind', choice('USE:', 'UNUSE:', 'REUSE:', 'IN:', 'QUALIFIED:')),
        field('vocabulary', $.name)),
      seq('QUALIFIED-WITH:', field('vocabulary', $.name), field('prefix', $.name)),
      seq(field('kind', choice('FROM:', 'EXCLUDE:')), field('vocabulary', $.name),
        '=>', repeat(field('name', $.name)), ';'),
      seq('RENAME:', field('original', $.name), field('vocabulary', $.name),
        '=>', field('name', $.name)),
    ),

    tuple_defn: $ => seq(field('kind', choice('TUPLE:', 'ERROR:', 'BUILTIN:')),
      field('name', $.name), optional(seq('<', field('superclass', $.name))),
      repeat(field('slot', choice($.name, $.slot))), ';'),
    slot: $ => seq('{', field('name', $.name), optional(field('type', $.name)),
      repeat($.slot_attribute), '}'),
    slot_attribute: $ => choice('read-only', seq('initial:', field('value', $._expression))),
    class_defn: $ => choice(
      seq(field('kind', choice('MIXIN:', 'SINGLETON:')), field('name', $.name)),
      seq(field('kind', choice('UNION:', 'INTERSECTION:')),
        field('name', $.name), repeat(field('member', $.name)), ';'),
      seq('SINGLETONS:', repeat(field('name', $.name)), ';'),
    ),
    predicate_defn: $ => seq('PREDICATE:', field('name', $.name), '<',
      field('superclass', $.name), optional(field('body', $.expression)), ';'),
    constructor_defn: $ => seq('C:', field('name', $.name), field('class', $.name)),
    instance_defn: $ => seq('INSTANCE:', field('class', $.name), field('mixin', $.name)),
    symbol_defn: $ => choice(
      seq(field('kind', choice('SYMBOL:', 'SLOT:')), field('name', $.name)),
      seq('SYMBOLS:', repeat(field('name', $.name)), ';'),
    ),
    constant_defn: $ => seq(field('kind', choice('CONSTANT:', 'INITIALIZED-SYMBOL:')),
      field('name', $.name), field('value', $._expression)),
    alias_defn: $ => seq('ALIAS:', field('name', $.name), field('target', $.name)),
    deferred_defn: $ => seq('DEFER:', field('name', $.name)),
    main_defn: $ => seq(field('kind', choice('MAIN:', 'STARTUP-HOOK:', 'SHUTDOWN-HOOK:')),
      field('entry', choice($.word, $.quotation))),
    private_section: $ => seq('<PRIVATE', repeat($._form), 'PRIVATE>'),
    compile_time: $ => seq('<<', repeat($._form), '>>'),

    expression: $ => repeat1($._expression),
    _expression: $ => choice(
      $.word, $.number, $.boolean, $.string, $.raw_string, $.regexp,
      $.character_literal, $.nan_literal, $.quotation, $.array,
      $.stack_effect, $.call_effect, $.word_literal, $.method_literal,
      $.postpone, $.local_binding, $.declaration,
    ),
    boolean: _ => choice('t', 'f'),
    character_literal: $ => seq('CHAR:', $.character),
    nan_literal: $ => seq('NAN:', field('payload', $.name)),
    quotation: $ => choice(
      seq(field('open', choice('[', "'[", '$[', '[let')),
        optional(field('body', $.expression)), ']'),
      seq('[|', field('parameters', $.local_parameters), '|',
        optional(field('body', $.expression)), ']'),
    ),
    local_parameters: $ => repeat1($.name),
    array: $ => seq(field('kind', $.collection_start), repeat($._expression), '}'),
    word_literal: $ => seq('\\', field('name', alias($._raw_name, $.name))),
    method_literal: $ => seq('M\\', field('class', $.name), field('name', alias($._raw_name, $.name))),
    postpone: $ => seq('POSTPONE:', field('name', alias($._raw_name, $.name))),
    declaration: _ => choice('inline', 'recursive', 'flushable', 'foldable',
      'delimiter', 'deprecated', 'final', 'auto-use'),
    local_binding: $ => seq(':>', choice(field('name', $.name),
      seq('(', repeat(field('name', $.name)), ')'))),

    stack_effect: $ => seq('(', optional(field('inputs', $.effect)), '--',
      optional(field('outputs', $.effect)), ')'),
    call_effect: $ => seq(field('kind', choice('call(', 'execute(')),
      optional(field('inputs', $.effect)), '--', optional(field('outputs', $.effect)), ')'),
    effect: $ => repeat1($.effect_parameter),
    effect_parameter: $ => choice(
      prec.right(seq(field('name', alias($._effect_name, $.name)),
        optional(seq(':', field('type', choice($.word, $.stack_effect, $.array)))))),
      seq(':', field('type', choice($.word, $.stack_effect, $.array))),
    ),
  },
});
