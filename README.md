# tree-sitter-factor

[![Test](https://github.com/mrjbq7/tree-sitter-factor/actions/workflows/test.yml/badge.svg)](https://github.com/mrjbq7/tree-sitter-factor/actions/workflows/test.yml)

A [Tree-sitter](https://tree-sitter.github.io/tree-sitter/) grammar for the
[Factor programming language](https://factorcode.org/), with structured syntax
trees and queries for syntax highlighting, local variables, symbol navigation,
folding, and indentation. Originally created by Razetime for use with Helix.

## Syntax coverage

- Word definitions, locals definitions, macros, memoized and typed words,
  methods, generics, hooks, parsing words, aliases, constants, and symbols.
- Vocabulary imports, qualified names, selective imports, renaming, private
  sections, compile-time forms, and lifecycle hooks.
- Tuples, inheritance, typed slots and attributes, classes, predicates, mixins,
  instances, and constructors.
- Quotations, lexical quotation parameters, local bindings, nested collections,
  vocabulary-defined collection and quotation prefixes, and word/method literals.
- Stack effects, nested quotation effects, typed parameters, row variables,
  and call/execute effects.
- Integer radices, floats, ratios, mixed numbers, complex numbers, booleans,
  character literals, strings, triple-quoted strings, raw strings with matching
  delimiters, regular expressions, comments, and shebangs.
- Standard-library syntax including FFI declarations, callbacks, structs,
  enums, functors, parser generators, documentation markup, named multiline
  strings, COM interfaces, and Objective-C classes.

The external C scanner recognizes complete Factor words, including punctuation
and Unicode names, and handles delimited text. Ordinary vocabulary words remain
ordinary words in the grammar; builtin highlighting is maintained separately.
Definitions expose fields such as `name`, `effect`, and `body` for editor tools.

Factor allows vocabularies to define new parsing words. This grammar models
standard syntax and selected standard-library extensions; it does not execute
arbitrary parsing words, resolve vocabulary imports, or validate stack effects.
Some standard-library directives use a generic definition-body representation.

## Editor queries

| Query | Purpose |
| --- | --- |
| `queries/highlights.scm` | Definitions, types, literals, syntax words, and builtin vocabulary words |
| `queries/locals.scm` | Scopes and bindings for locals definitions, lexical quotations, and local bindings |
| `queries/tags.scm` | Function, method, and class definitions, plus call references |
| `queries/folds.scm` | Definitions, sections, quotations, collections, and multiline text |
| `queries/indents.scm` | Block indentation and closing delimiters |

The highlighting data contains 1,240 distinct builtin words from 31 vocabulary
lists maintained by [factor.vim](https://github.com/factor/factor.vim).
Builtin captures use local-variable predicates so local bindings can take
precedence in editors that support them. Query integration and indentation
capture conventions depend on the editor.

The Rust binding exports `HIGHLIGHTS_QUERY`, `LOCALS_QUERY`, `TAGS_QUERY`,
`FOLDS_QUERY`, `INDENTS_QUERY`, and `NODE_TYPES` alongside `LANGUAGE`.

## Build and test

Install Rust and a C compiler, then run from the repository root:

```sh
cargo test
```

This compiles the checked-in parser and scanner, checks all 83 corpus cases
including malformed input, compiles all five editor queries, checks definition
fields and nested effects, and runs the Rust documentation example.
Once dependencies are cached, `cargo test --offline` works too.

[GitHub Actions](.github/workflows/test.yml) runs Rust tests, documentation tests,
and generated-file checks on Linux, macOS, and Windows. Node.js, npm, and
`node_modules` are not required for this workflow.

## Regenerate the parser

The project uses Tree-sitter CLI 0.27.0 and generates parser ABI 15. The Rust
binding is tested with Tree-sitter 0.27. The CLI's native JavaScript runtime
can evaluate `grammar.js` without Node.js.

```sh
cargo install tree-sitter-cli --version 0.27.0 --locked
tree-sitter generate --js-runtime native
cargo run --example generate
tree-sitter test
```

Commit the generated files alongside grammar changes. The Rust generator keeps
`src/reserved.h` synchronized with `src/grammar.json` and produces the builtin
highlighting query from `queries/highlights-base.scm` and `data/builtins.json`.
Edit those inputs rather than the generated files. Check for stale outputs with:

```sh
cargo run --example generate -- --check
```

Refresh builtin vocabulary data from a local factor.vim checkout with:

```sh
cargo run --example generate -- --import-builtins ../factor.vim/syntax/factor/generated.vim
```

For incremental parsing stress tests, the standalone CLI also provides:

```sh
tree-sitter fuzz --iterations 100 --edits 5
```

## Rust binding

Use this checkout as a path dependency, together with `tree-sitter = "0.27"`:

```toml
[dependencies]
tree-sitter = "0.27"
tree-sitter-factor = { path = "../tree-sitter-factor" }
```

```rust
let mut parser = tree_sitter::Parser::new();
parser
    .set_language(&tree_sitter_factor::LANGUAGE.into())
    .expect("Error loading Factor parser");
let tree = parser.parse(": square ( x -- y ) dup * ;", None).unwrap();
assert!(!tree.root_node().has_error());
```

## Optional Node binding

The repository also retains npm packaging and a Node binding for JavaScript
applications. Its runtime dependency is `tree-sitter` 0.25.x, independently of
the Rust runtime and CLI versions. To develop or test this binding, install
Node.js 20 or newer and native build tools, then run:

```sh
npm ci
npm run test:bindings
```

```javascript
const Parser = require('tree-sitter');
const Factor = require('./bindings/node');
const parser = new Parser();
parser.setLanguage(Factor);
const tree = parser.parse(': square ( x -- y ) dup * ;');
```

These commands are optional. `node_modules` and native Node build artifacts
are ignored by Git. `npm test` delegates to the primary Rust test suite.

## References and license

- [Factor documentation](https://docs.factorcode.org/)
- [Factor source and standard library](https://github.com/factor/factor)
- [factor.vim syntax highlighting](https://github.com/factor/factor.vim)
- [Tree-sitter documentation](https://tree-sitter.github.io/tree-sitter/)

Distributed under the [ISC license](LICENSE).
