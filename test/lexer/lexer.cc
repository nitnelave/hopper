#include <string>

#include "error/error.h"
#include "lexer/lexer.h"
#include "test_utils/lexing.h"
#include "test_utils/utils.h"

namespace lexer {
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

// Misc.
TEST(LexerTest, LexErrorToStream) {
  Range r{"file", 1, 2, 3, 4};
  LexError error{"test", r};
  std::stringstream ss;
  ss << error;
  EXPECT_EQ("test in " + r.to_string(), ss.str());
}

TEST(LexerTest, InvalidSourceTag) {
  auto fake_tag = static_cast<Lexer::SourceTag>(-1);
  EXPECT_THROW(Lexer("test", fake_tag), std::domain_error);
}

TEST(LexerTest, TokenTypeToStream) {
  auto tt = TokenType::PLUS;
  std::stringstream ss;
  ss << tt;
  EXPECT_EQ("PLUS", ss.str());
}

// Tests of binary operators.
TEST(LexerTest, BinaryOperators) {
  EXPECT_TRUE(compare_tokens_types_and_symbols(
      "+ - / * div mod ?: ?. ?->",
      {TokenType::PLUS, TokenType::MINUS, TokenType::DIVIDE, TokenType::STAR,
       TokenType::DIV, TokenType::MODULO, TokenType::QUESTION_MARK_COLON,
       TokenType::QUESTION_MARK_DOT, TokenType::QUESTION_MARK_ARROW}));
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
  EXPECT_TRUE(compare_tokens_types_and_symbols(
      "?-*", {TokenType::QUESTION_MARK, TokenType::MINUS, TokenType::STAR}));
}

TEST(LexerTest, BinaryOperatorsNoSpace) {
  EXPECT_TRUE(compare_tokens_types_and_symbols(
      "+-/*?:?.?->",
      {TokenType::PLUS, TokenType::MINUS, TokenType::DIVIDE, TokenType::STAR,
       TokenType::QUESTION_MARK_COLON, TokenType::QUESTION_MARK_DOT,
       TokenType::QUESTION_MARK_ARROW}));
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

// Tests of delimiters.
TEST(LexerTest, Delimiters) {
  EXPECT_TRUE(compare_tokens_types_and_symbols(
      "( ) [ ] { } ; : , -> :: ::: . .. ...",
      {TokenType::OPEN_PAREN, TokenType::CLOSE_PAREN, TokenType::OPEN_BRACKET,
       TokenType::CLOSE_BRACKET, TokenType::OPEN_BRACE, TokenType::CLOSE_BRACE,
       TokenType::SEMICOLON, TokenType::COLON, TokenType::COMMA,
       TokenType::ARROW, TokenType::COLON_COLON, TokenType::COLON_COLON,
       TokenType::COLON, TokenType::DOT, TokenType::DOTDOT,
       TokenType::DOTDOTDOT}));
  EXPECT_TRUE(compare_tokens_types_and_symbols(
      "()[]{};:->,",
      {TokenType::OPEN_PAREN, TokenType::CLOSE_PAREN, TokenType::OPEN_BRACKET,
       TokenType::CLOSE_BRACKET, TokenType::OPEN_BRACE, TokenType::CLOSE_BRACE,
       TokenType::SEMICOLON, TokenType::COLON, TokenType::ARROW,
       TokenType::COMMA}));
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
  EXPECT_TRUE(compare_token_types(
      "0+0 + 1", {TokenType::INT, TokenType::PLUS, TokenType::INT,
                  TokenType::PLUS, TokenType::INT},
      {"0", "+", "0", "+", "1"}));
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
      "_ abstract as baseclass break catch class constant continue dataclass "
      "default defer div do else enum extern finally for forward from fun if "
      "implements import in inherit interface is macro mod mut new noexcept "
      "object override package protected private public pure reinterpret_cast "
      "return throw try try_compile using val virtual when while with yield",
      {TokenType::UNDERSCORE, TokenType::ABSTRACT,
       TokenType::AS,         TokenType::BASECLASS,
       TokenType::BREAK,      TokenType::CATCH,
       TokenType::CLASS,      TokenType::CONSTANT,
       TokenType::CONTINUE,   TokenType::DATACLASS,
       TokenType::DEFAULT,    TokenType::DEFER,
       TokenType::DIV,        TokenType::DO,
       TokenType::ELSE,       TokenType::ENUM,
       TokenType::EXTERN,     TokenType::FINALLY,
       TokenType::FOR,        TokenType::FORWARD,
       TokenType::FROM,       TokenType::FUN,
       TokenType::IF,         TokenType::IMPLEMENTS,
       TokenType::IMPORT,     TokenType::IN,
       TokenType::INHERIT,    TokenType::INTERFACE,
       TokenType::IS,         TokenType::MACRO,
       TokenType::MODULO,     TokenType::MUT,
       TokenType::NEW,        TokenType::NOEXCEPT,
       TokenType::OBJECT,     TokenType::OVERRIDE,
       TokenType::PACKAGE,    TokenType::PROTECTED,
       TokenType::PRIVATE,    TokenType::PUBLIC,
       TokenType::PURE,       TokenType::REINTERPRET_CAST,
       TokenType::RETURN,     TokenType::THROW,
       TokenType::TRY,        TokenType::TRY_COMPILE,
       TokenType::USING,      TokenType::VAL,
       TokenType::VIRTUAL,    TokenType::WHEN,
       TokenType::WHILE,      TokenType::WITH,
       TokenType::YIELD}));
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
  auto actual_columns =
      MAP_VEC(tokens, (std::pair<int, int>{__ARG__.location.begin.column,
                                           __ARG__.location.end.column}));
  EXPECT_EQ(expected_columns, actual_columns);
}
}  // namespace lexer
