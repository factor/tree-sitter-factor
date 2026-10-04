use std::{fs, path::Path};

// Tree-sitter corpus expectations omit field labels and format trees freely.
fn normalize(tree: &str) -> String {
    tree.split_whitespace()
        .filter(|token| !token.ends_with(':'))
        .collect::<String>()
}

#[test]
fn grammar_corpus() {
    let directory = Path::new(env!("CARGO_MANIFEST_DIR")).join("test/corpus");
    let mut files: Vec<_> = fs::read_dir(directory)
        .unwrap()
        .map(|entry| entry.unwrap().path())
        .filter(|path| path.extension().is_some_and(|extension| extension == "txt"))
        .collect();
    files.sort();
    let mut parser = tree_sitter::Parser::new();
    parser.set_language(&crate::LANGUAGE.into()).unwrap();
    let mut count = 0;
    let mut failures = Vec::new();
    let separator = "========================================================================";
    for file in files {
        let contents = fs::read_to_string(&file).unwrap();
        let sections: Vec<_> = contents.split(separator).collect();
        assert_eq!(
            sections.len() % 2,
            1,
            "Malformed corpus: {}",
            file.display()
        );
        for case in sections[1..].chunks_exact(2) {
            let name = case[0].trim();
            let (source, expected) = case[1]
                .split_once("\n---\n")
                .unwrap_or_else(|| panic!("Missing expectation: {}: {name}", file.display()));
            // Match the CLI's removal of the blank lines surrounding each case.
            let source = source.trim_matches('\n');
            let tree = parser.parse(source, None).unwrap();
            let actual = tree.root_node().to_sexp();
            count += 1;
            if normalize(&actual) != normalize(expected) {
                failures.push(format!(
                    "{}: {name}\nExpected: {}\nActual: {actual}",
                    file.display(),
                    expected.trim()
                ));
            }
        }
    }
    assert!(count > 0, "No grammar corpus cases found");
    assert!(
        failures.is_empty(),
        "{} of {count} corpus cases failed:\n{}",
        failures.len(),
        failures.join("\n\n")
    );
    println!("Passed {count} grammar corpus cases");
}
