fn main() {
    cc::Build::new()
        .include("src")
        .file("src/parser.c")
        .file("src/scanner.c")
        .flag_if_supported("-std=c11")
        .compile("tree-sitter-factor");
    for path in ["src/parser.c", "src/scanner.c", "src/reserved.h", "src/tree_sitter/parser.h"] {
        println!("cargo:rerun-if-changed={path}");
    }
}
