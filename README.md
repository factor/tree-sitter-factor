# tree-sitter-factor
A tree-sitter grammar for the [Factor](https://factorcode.org/) programming language.

Build and test with Rust; Node.js and `node_modules` are optional:

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
tree-sitter test
```

Node dependencies are needed only for the Node binding and Node helper scripts.

Made originally for use with [Helix editor](https://helix-editor.com/).
Factor has an [Atom package](https://github.com/factor/atom-language-factor) that does not use this grammar.
