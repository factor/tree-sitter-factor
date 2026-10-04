(source_file) @local.scope
(word_defn) @local.scope
(method_defn) @local.scope
(quotation) @local.scope

; Only locals definitions bind their input effect variables.
(word_defn
  kind: [ "::" "MACRO::" "MEMO::" "IDENTITY-MEMO::" "TYPED::" ]
  effect: (stack_effect inputs: (effect (effect_parameter name: (name) @local.definition))))
(method_defn "M::"
  effect: (stack_effect inputs: (effect (effect_parameter name: (name) @local.definition))))
(local_parameters (name) @local.definition)
(local_binding name: (name) @local.definition)
(word) @local.reference
