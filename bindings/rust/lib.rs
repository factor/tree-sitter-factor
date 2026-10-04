//! This crate provides Factor language support for the [tree-sitter] parsing library.
//!
//! Typically, you will use the [`LANGUAGE`] constant to add this language to a
//! tree-sitter [`Parser`], and then use the parser to parse some code:
//!
//! ```
//! let code = ": square ( x -- y ) dup * ;";
//! let mut parser = tree_sitter::Parser::new();
//! let language = tree_sitter_factor::LANGUAGE;
//! parser
//!     .set_language(&language.into())
//!     .expect("Error loading Factor parser");
//! let tree = parser.parse(code, None).unwrap();
//! assert!(!tree.root_node().has_error());
//! ```
//!
//! [`Parser`]: https://docs.rs/tree-sitter/0.27/tree_sitter/struct.Parser.html
//! [tree-sitter]: https://tree-sitter.github.io/

use tree_sitter_language::LanguageFn;

unsafe extern "C" {
    fn tree_sitter_factor() -> *const ();
}

/// The tree-sitter [`LanguageFn`] for this grammar.
pub const LANGUAGE: LanguageFn = unsafe { LanguageFn::from_raw(tree_sitter_factor) };

/// The content of the [`node-types.json`] file for this grammar.
///
/// [`node-types.json`]: https://tree-sitter.github.io/tree-sitter/using-parsers/6-static-node-types
pub const NODE_TYPES: &str = include_str!("../../src/node-types.json");

/// The syntax highlighting query for this grammar.
pub const HIGHLIGHTS_QUERY: &str = include_str!("../../queries/highlights.scm");

/// The local variable query for this grammar.
pub const LOCALS_QUERY: &str = include_str!("../../queries/locals.scm");

/// The symbol navigation query for this grammar.
pub const TAGS_QUERY: &str = include_str!("../../queries/tags.scm");

/// The folding query for this grammar.
pub const FOLDS_QUERY: &str = include_str!("../../queries/folds.scm");

/// The indentation query for this grammar.
pub const INDENTS_QUERY: &str = include_str!("../../queries/indents.scm");

#[cfg(test)]
mod corpus;

#[cfg(test)]
mod tests {
    #[test]
    fn queries_compile() {
        let language = super::LANGUAGE.into();
        tree_sitter::Query::new(&language, super::HIGHLIGHTS_QUERY).unwrap();
        tree_sitter::Query::new(&language, super::LOCALS_QUERY).unwrap();
        tree_sitter::Query::new(&language, super::TAGS_QUERY).unwrap();
        tree_sitter::Query::new(&language, super::FOLDS_QUERY).unwrap();
        tree_sitter::Query::new(&language, super::INDENTS_QUERY).unwrap();
    }

    #[test]
    fn definitions_have_stable_fields_and_nested_effects() {
        let mut parser = tree_sitter::Parser::new();
        parser.set_language(&super::LANGUAGE.into()).unwrap();
        let source = ": twice ( quot: ( x -- y ) x -- y ) call ;";
        let tree = parser.parse(source, None).unwrap();
        assert!(!tree.root_node().has_error());
        let definition = tree.root_node().named_child(0).unwrap();
        let name = definition.child_by_field_name("name").unwrap();
        assert_eq!(name.utf8_text(source.as_bytes()).unwrap(), "twice");
        let effect = definition.child_by_field_name("effect").unwrap();
        let inputs = effect.child_by_field_name("inputs").unwrap();
        assert_eq!(inputs.named_child_count(), 2);
        let typed = inputs.named_child(0).unwrap();
        assert_eq!(typed.child_by_field_name("type").unwrap().kind(), "stack_effect");
        assert_eq!(definition.child_by_field_name("body").unwrap().kind(), "expression");
    }

    #[test]
    fn test_can_load_grammar() {
        let mut parser = tree_sitter::Parser::new();
        parser
            .set_language(&super::LANGUAGE.into())
            .expect("Error loading Factor parser");
    }
}
