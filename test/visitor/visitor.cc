#include "ast/ast.h"
#include "ast/module.h"
#include "parser/parser.h"
#include "test_utils/utils.h"

// This test is mainly for coverage. All the parsed AST structures should be
// represented in the test, and the coverage should correspond.

const std::string sample_program = R"(
val a : Int = 3;
fun test() = 3 + b;
fun test2() : Int {
  return true;
}
fun test3() {
  test();
  return;
}
)";

TEST(Visitor, TestVisitor) {
  lexer::Lexer lexer = lexer::from_string(sample_program);
  parser::Parser parser(&lexer);
  auto res = parser.parse();
  EXPECT_EQ(true, res.is_ok());
  ast::ASTVisitor visitor;
  res.value_or_die()->accept(visitor);
}
