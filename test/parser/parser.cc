#include "parser/parser.h"
#include "test_utils/utils.h"

namespace parser {

TEST(ParseError, Output) {
  lexer::Range range("file", 1, 2, 3, 4);
  ParseError error("message", range);
  EXPECT_EQ("message\n At file from 1:2 to 3:4", error.to_string());
}

}  // namespace parser
