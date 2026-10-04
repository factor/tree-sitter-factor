# tree-sitter-factor
A tree-sitter grammar for the [Factor](https://factorcode.org/) programming language.

Build and test with Rust:

```sh
cargo test
```

This compiles the checked-in C parser and external scanner, runs every case in
`test/corpus`, and checks the Rust binding and highlighting/local queries.
After dependencies have been downloaded, `cargo test --offline` also works.

To regenerate the parser or run the Tree-sitter CLI's test tooling without npm:

```sh
cargo install tree-sitter-cli --version 0.27.0 --locked
tree-sitter generate --js-runtime native
cargo run --example generate
tree-sitter test
```

Node.js and npm are not needed. `grammar.js` is evaluated by Tree-sitter's
native JavaScript runtime. The Rust generator updates the scanner's reserved
words and highlighting query; `cargo run --example generate -- --check` checks
that these generated files are current.

To refresh builtin highlighting from the vocabulary lists in factor.vim:

```sh
cargo run --example generate -- --import-builtins ../factor.vim/syntax/factor/generated.vim
```

Made originally for use with [Helix editor](https://helix-editor.com/).
Factor has an [Atom package](https://github.com/factor/atom-language-factor) that does not use this grammar.
