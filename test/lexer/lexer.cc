#include <string>

#include "error/error.h"
#include "lexer/lexer.h"
#include "test_utils/lexing.h"
#include "test_utils/utils.h"

using namespace lexer;

namespace {

/// Compare a list of tokens with expected types, and expected text if given.
testing::AssertionResult compare_token_types(
    const std::vector<Token>& tokens, const std::vector<TokenType>& types,
    const std::vector<std::string>& text = {}) {
  CALL_ASSERT(compare(types, MAP_VEC(tokens, __ARG__.type)));
  if (!text.empty()) CALL_ASSERT(compare(text, MAP_VEC(tokens, __ARG__.text)));
  return testing::AssertionSuccess();
}

/// Lex a string, then compare the tokens with expected types and text.
testing::AssertionResult compare_token_types(
    const std::string& input, const std::vector<TokenType>& types,
    const std::vector<std::string>& text = {}) {
  auto tokens_or = string_to_tokens(input);
  if (!tokens_or.is_ok())
    return testing::AssertionFailure() << tokens_or.error_or_die();
  const auto& tokens = tokens_or.value_or_die();
  return compare_token_types(tokens, types, text);
}

/// Lex a string, then compare the tokens with the expected types and the
/// canonical representation of the types.
testing::AssertionResult compare_tokens_types_and_symbols(
    const std::string& input, const std::vector<TokenType>& types) {
  auto tokens_or = string_to_tokens(input);
  if (!tokens_or.is_ok())
    return testing::AssertionFailure() << tokens_or.error_or_die();
  const auto& tokens = tokens_or.value_or_die();
  return compare_token_types(tokens, types, MAP_VEC(types, to_symbol(__ARG__)));
}

}  // namespace

// Tests of binary operators.
TEST(LexerTest, BinaryOperators) {
  EXPECT_TRUE(compare_tokens_types_and_symbols(
      "+ - / *",
      {TokenType::PLUS, TokenType::MINUS, TokenType::DIVIDE, TokenType::STAR}));
  EXPECT_TRUE(compare_tokens_types_and_symbols(
      "|| && > < >= <=",
      {TokenType::OR, TokenType::AND, TokenType::GREATER, TokenType::LESS,
       TokenType::GREATER_OR_EQUAL, TokenType::LESS_OR_EQUAL}));
  EXPECT_TRUE(compare_tokens_types_and_symbols(
      "== !=", {TokenType::EQUAL, TokenType::DIFFERENT}));
  EXPECT_TRUE(compare_tokens_types_and_symbols(
      "|> <| | ^", {TokenType::BITSHIFT_RIGHT, TokenType::BITSHIFT_LEFT,
                    TokenType::BITOR, TokenType::BITXOR}));
  EXPECT_TRUE(compare_tokens_types_and_symbols(
      "= += -= *= /= |= ^= &=",
      {TokenType::ASSIGN, TokenType::PLUS_ASSIGN, TokenType::MINUS_ASSIGN,
       TokenType::TIMES_ASSIGN, TokenType::DIVIDE_ASSIGN, TokenType::OR_ASSIGN,
       TokenType::XOR_ASSIGN, TokenType::AND_ASSIGN}));
}

TEST(LexerTest, BinaryOperatorsNoSpace) {
  EXPECT_TRUE(compare_tokens_types_and_symbols(
      "+-/*",
      {TokenType::PLUS, TokenType::MINUS, TokenType::DIVIDE, TokenType::STAR}));
  EXPECT_TRUE(compare_tokens_types_and_symbols(
      "||&&><>=<=",
      {TokenType::OR, TokenType::AND, TokenType::GREATER, TokenType::LESS,
       TokenType::GREATER_OR_EQUAL, TokenType::LESS_OR_EQUAL}));
  EXPECT_TRUE(compare_tokens_types_and_symbols(
      "==!=", {TokenType::EQUAL, TokenType::DIFFERENT}));
  EXPECT_TRUE(compare_tokens_types_and_symbols(
      "|><||^", {TokenType::BITSHIFT_RIGHT, TokenType::BITSHIFT_LEFT,
                 TokenType::BITOR, TokenType::BITXOR}));
  EXPECT_TRUE(compare_tokens_types_and_symbols(
      "=+=-=*=/=|=^=&=",
      {TokenType::ASSIGN, TokenType::PLUS_ASSIGN, TokenType::MINUS_ASSIGN,
       TokenType::TIMES_ASSIGN, TokenType::DIVIDE_ASSIGN, TokenType::OR_ASSIGN,
       TokenType::XOR_ASSIGN, TokenType::AND_ASSIGN}));
}

// Tests of unary operators.
TEST(LexerTest, UnaryOperators) {
  EXPECT_TRUE(compare_tokens_types_and_symbols(
      "& ! ~ ++ --", {TokenType::AMPERSAND, TokenType::BANG, TokenType::TILDE,
                      TokenType::INCREMENT, TokenType::DECREMENT}));
  EXPECT_TRUE(compare_tokens_types_and_symbols(
      "&!~++--", {TokenType::AMPERSAND, TokenType::BANG, TokenType::TILDE,
                  TokenType::INCREMENT, TokenType::DECREMENT}));
  EXPECT_TRUE(compare_tokens_types_and_symbols(
      "& = ! =", {TokenType::AMPERSAND, TokenType::ASSIGN, TokenType::BANG,
                  TokenType::ASSIGN}));
}

// Tests of numbers.
TEST(LexerTest, Numbers) {
  EXPECT_TRUE(compare_token_types(
      "1 435 0x1Ab 0o745 0b100101 -3 1+1",
      {TokenType::INT, TokenType::INT, TokenType::HEX, TokenType::OCT,
       TokenType::BINARY_NUMBER, TokenType::MINUS, TokenType::INT,
       TokenType::INT, TokenType::PLUS, TokenType::INT},
      // Value in base 10 of the literals.
      {"1", "435", "427", "485", "37", "-", "3", "1", "+", "1"}));
}

// Tests of numbers.
TEST(LexerTest, NumbersFail) {
  const auto& res = string_to_tokens("0f3");
  EXPECT_FALSE(res.is_ok());
  EXPECT_EQ(LexError("Invalid number literal", make_range(1, 1, 1, 3)),
            res.error_or_die());
}

// Tests of comments.
TEST(LexerTest, Comments) {
  EXPECT_TRUE(compare_token_types(
      "1+1 // Test: 4 5\n2", {TokenType::INT, TokenType::PLUS, TokenType::INT,
                              TokenType::COMMENT, TokenType::INT},
      {"1", "+", "1", "// Test: 4 5", "2"}));
}

// Tests of keywords.
TEST(LexerTest, Keywords) {
  EXPECT_TRUE(compare_tokens_types_and_symbols(
      "as do enum data class private public from for forward fun if import is "
      "mut return val when while",
      {TokenType::AS, TokenType::DO, TokenType::ENUM, TokenType::DATA,
       TokenType::CLASS, TokenType::PRIVATE, TokenType::PUBLIC, TokenType::FROM,
       TokenType::FOR, TokenType::FORWARD, TokenType::FUN, TokenType::IF,
       TokenType::IMPORT, TokenType::IS, TokenType::MUT, TokenType::RETURN,
       TokenType::VAL, TokenType::WHEN, TokenType::WHILE}));
}

// Tests of identifiers.
TEST(LexerTest, Identifiers) {
  EXPECT_TRUE(compare_token_types(
      "a A ab Ab a_b A_b ABC",
      {TokenType::LOWER_CASE_IDENT, TokenType::UPPER_CASE_IDENT,
       TokenType::LOWER_CASE_IDENT, TokenType::UPPER_CASE_IDENT,
       TokenType::LOWER_CASE_IDENT, TokenType::UPPER_CASE_IDENT,
       TokenType::UPPER_CASE_IDENT}));
}

TEST(LexerTest, FunctionDeclaration) {
  EXPECT_TRUE(compare_token_types(
      "fun test(mut a : Int, val b : Char*) : Int { return 4; }",
      {TokenType::FUN,
       TokenType::LOWER_CASE_IDENT,
       TokenType::OPEN_PAREN,
       TokenType::MUT,
       TokenType::LOWER_CASE_IDENT,
       TokenType::COLON,
       TokenType::UPPER_CASE_IDENT,
       TokenType::COMMA,
       TokenType::VAL,
       TokenType::LOWER_CASE_IDENT,
       TokenType::COLON,
       TokenType::UPPER_CASE_IDENT,
       TokenType::STAR,
       TokenType::CLOSE_PAREN,
       TokenType::COLON,
       TokenType::UPPER_CASE_IDENT,
       TokenType::OPEN_BRACE,
       TokenType::RETURN,
       TokenType::INT,
       TokenType::SEMICOLON,
       TokenType::CLOSE_BRACE},
      {"fun",  "test", "(", "mut", "a",   ":", "Int",    ",", "val", "b", ":",
       "Char", "*",    ")", ":",   "Int", "{", "return", "4", ";",   "}"}));
}

// Test of location.
TEST(LexerTest, Location) {
  auto tokens_or =
      string_to_tokens("1 12 123 1234 1+1 1++1 abc 0b110 // comment\n2");
  ASSERT_TRUE(tokens_or.is_ok());
  const auto& tokens = tokens_or.value_or_die();
  std::vector<std::pair<int, int>> expected_columns = {
      {1, 1},   {3, 4},   {6, 8},   {10, 13}, {15, 15}, {16, 16}, {17, 17},
      {19, 19}, {20, 21}, {22, 22}, {24, 26}, {28, 32}, {34, 43}, {1, 1},
  };
  ASSERT_EQ(expected_columns.size(), tokens.size());
  for (unsigned int i = 0; i < expected_columns.size() - 1; ++i) {
    EXPECT_EQ("<string>", tokens[i].location.file);
    EXPECT_EQ(1, tokens[i].location.begin.line);
    EXPECT_EQ(1, tokens[i].location.end.line);
  }
  EXPECT_EQ(2, tokens.back().location.begin.line);
  EXPECT_EQ(2, tokens.back().location.end.line);
  auto actual_columns = MAP_VEC(
      tokens, (std::pair<int, int>{__ARG__.location.begin.column, __ARG__.location.end.column}));
  EXPECT_EQ(expected_columns, actual_columns);
}
