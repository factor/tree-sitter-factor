(number) @number
(boolean) @constant.builtin
(string) @string
(raw_string) @string
(regexp) @string.regexp
(character_literal) @character
(comment) @comment
(shebang) @comment
(word_defn name: (name) @function)
(method_defn class: (name) @type name: (name) @function.method)
(generic_defn name: (name) @function)
(syntax_defn name: (name) @function.macro)
(effect_parameter name: (name) @variable.parameter)
