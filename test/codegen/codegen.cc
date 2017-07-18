#include "codegen/codegen.h"
#include "ast/module.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "test_utils/utils.h"

TEST(Codegen, TargetUninitializedError) {
  auto lexer = lexer::from_string("");
  auto parser = parser::Parser(&lexer);
  auto result = parser.parse();
  EXPECT_TRUE(result.is_ok()) << "Empty parse failed";
  EXPECT_THROW(codegen::CodeGenerator generator("<string>"),
               std::runtime_error);
}
