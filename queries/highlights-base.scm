; General values and vocabulary words.
(word) @function.call
(name) @variable
(number) @number
(boolean) @constant.builtin
(nan_literal) @number
(alien_literal) @constant
(string) @string
(raw_string) @string
(string_body) @string
(regexp) @string.regexp
(character_literal) @character
(comment) @comment
(shebang) @comment

; Definitions and declarations.
(word_defn name: (name) @function)
(method_defn class: (name) @type name: (name) @function.method)
(generic_defn name: (name) @function)
(syntax_defn name: (name) @function.macro)
(constructor_defn name: (name) @function)
(constructor_defn class: (name) @type)
(deferred_defn name: (name) @function)
(alias_defn name: (name) @function target: (name) @function)
(tuple_defn name: (name) @type)
(tuple_defn superclass: (name) @type)
(tuple_defn slot: (name) @property)
(class_defn name: (name) @type)
(class_defn member: (name) @type)
(predicate_defn name: (name) @type superclass: (name) @type)
(instance_defn class: (name) @type mixin: (name) @type)
(struct_defn name: (name) @type)
(enum_defn name: (name) @type)
(enum_defn type: (name) @type)
(enum_member (name) @constant)
(slot name: (name) @property)
(slot type: (name) @type)
(constant_defn name: (name) @constant)
(symbol_defn name: (name) @constant)
(named_string_defn name: (name) @function)
(ebnf_defn name: (name) @function)
(directive_defn name: (name) @function.macro)
(functor_defn name: (name) @function.macro)
(functor_binding name: (name) @variable)
(declaration) @attribute

; Imports and word wrappers.
(import_defn vocabulary: (name) @module)
(import_defn prefix: (name) @module)
(import_defn original: (name) @function)
(import_defn name: (name) @function)
(vocabulary_literal name: (name) @module)
(word_literal name: (name) @function)
(method_literal class: (name) @type name: (name) @function.method)
(postpone name: (name) @function)
(help_defn name: (name) @function)

; Stack effects and lexical variables.
(effect_parameter name: (name) @variable.parameter)
(effect_parameter type: (word) @type)
(local_parameters (name) @variable.parameter)
(local_binding name: (name) @variable)
(effect_start) @function.builtin

; Foreign declarations.
(ffi_defn name: (name) @function)
(ffi_defn return_type: (name) @type)
(ffi_defn type: (name) @type)
(ffi_defn foreign_name: (name) @function)
(ffi_parameter type: (name) @type name: (name) @variable.parameter)
(com_interface name: (name) @type superclass: (name) @type)
(com_method return_type: (name) @type name: (name) @function.method)
(objc_class name: (name) @type superclass: (name) @type)
(objc_method return_type: (name) @type name: (name) @function.method)
(variadic) @punctuation.special

; Delimiters, including vocabulary-defined quotation and collection prefixes.
[ "[" "]" "{" "}" "(" ")" "[|" "[let" "'[" "$[" ] @punctuation.bracket
(collection_start) @punctuation.bracket
(quotation_start) @punctuation.bracket
[ ";" "|" "--" "=>" ] @punctuation.delimiter
(effect_parameter ":" @punctuation.delimiter)
[ "\\" "M\\" "$" ":>" ] @operator
(slot_attribute [ "read-only" "initial:" "bits:" ] @attribute)

; Known parsing words. Ordinary builtins are handled by the generated section.
[
  ","
  ":"
  "::"
  ";CLASS>"
  ";FUNCTOR>"
  "<<"
  "<CLASS:"
  "<FUNCTOR:"
  "<PRIVATE"
  "<WATCH"
  ">>"
  "ABOUT:"
  "AFTER:"
  "ALIAS:"
  "ALIEN:"
  "ARTICLE:"
  "BE-PACKED-STRUCT:"
  "BE-STRUCT:"
  "BEFORE:"
  "BROADCAST:"
  "BUILTIN:"
  "C-GLOBAL:"
  "C-LIBRARY:"
  "C-TYPE:"
  "C:"
  "CALLBACK:"
  "CATEGORY-NOT:"
  "CATEGORY:"
  "CHAR:"
  "CHLOE:"
  "CLASS-METHOD:"
  "COCOA-PROTOCOL:"
  "COM-INTERFACE:"
  "COMPONENT:"
  "CONSTANT:"
  "CONSTRUCTOR:"
  "CONSULT:"
  "DEFER:"
  "DEFERS"
  "DEFINES"
  "DEFINES-CLASS"
  "DEFINES-PRIVATE"
  "DESTRUCTOR:"
  "EBNF:"
  "ENUM:"
  "ERROR:"
  "EXCLUDE:"
  "FLUSHABLE-INSN:"
  "FOLDABLE-INSN:"
  "FOREIGN-RECORD-TYPE:"
  "FOREIGN-RECORDS:"
  "FORGET:"
  "FROM:"
  "FUNCTION-ALIAS:"
  "FUNCTION:"
  "FUNCTOR-SYNTAX:"
  "GENERIC#:"
  "GENERIC:"
  "GIR:"
  "GL-FUNCTION:"
  "HELP:"
  "HI-REGISTERS:"
  "HINTS:"
  "HOOK:"
  "IDENTITY-MEMO:"
  "IDENTITY-MEMO::"
  "IMPLEMENT-STRUCTS:"
  "IN:"
  "INITIALIZE-ALIEN:"
  "INITIALIZED-SYMBOL:"
  "INSN:"
  "INSTANCE:"
  "INTERSECTION:"
  "IS"
  "LAZY:"
  "LAZY::"
  "LE-PACKED-STRUCT:"
  "LE-STRUCT:"
  "LIBRARY:"
  "M:"
  "M::"
  "MACRO:"
  "MACRO::"
  "MAIN-WINDOW:"
  "MAIN:"
  "MATCH-VARS:"
  "MATH:"
  "MEMO:"
  "MEMO::"
  "METHOD-CHAIN:"
  "METHOD:"
  "MIXIN:"
  "NAN:"
  "PACKED-STRUCT:"
  "PARTIAL-EBNF:"
  "PARTIAL-PEG:"
  "PEG:"
  "POSTPONE:"
  "PREDICATE:"
  "PRIMITIVE:"
  "PRIVATE>"
  "PROTOCOL:"
  "QUALIFIED-WITH:"
  "QUALIFIED:"
  "REGISTERS:"
  "RENAME:"
  "REUSE:"
  "RULE:"
  "SHUTDOWN-HOOK:"
  "SINGLETON:"
  "SINGLETONS:"
  "SKIP-DEFINITIONS:"
  "SLOT-PROTOCOL:"
  "SLOT:"
  "SPECIALIZED-ARRAYS:"
  "SPECIALIZED-VECTORS:"
  "STARTUP-HOOK:"
  "STRING:"
  "STRUCT:"
  "SYMBOL:"
  "SYMBOLS:"
  "SYNTAX:"
  "TAG:"
  "TAGS:"
  "TIP:"
  "TR:"
  "TUPLE:"
  "TYPED:"
  "TYPED::"
  "TYPEDEF:"
  "UNION-STRUCT:"
  "UNION:"
  "UNUSE:"
  "USE:"
  "USING:"
  "VOCAB:"
  "VREG-INSN:"
  "WATCH>"
  "WHERE"
  "X-FUNCTION:"
  "XML-ERROR:"
  "XML-NS:"
  "auto-use"
  "delimiter"
  "deprecated"
  "final"
  "flushable"
  "foldable"
  "inline"
  "recursive"
] @keyword

(ERROR) @error
