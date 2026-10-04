const assert = require("node:assert/strict");
const { test } = require("node:test");
const Parser = require("tree-sitter");
const Factor = require("./");
test("loads the Factor language and parses a definition", () => {
  const parser = new Parser();
  parser.setLanguage(Factor);
  const tree = parser.parse(": square ( x -- y ) dup * ;");
  assert.equal(tree.rootNode.hasError, false);
  assert.equal(tree.rootNode.firstNamedChild.type, "word_defn");
});
