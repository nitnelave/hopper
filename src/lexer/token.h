#pragma once

#include <string>

namespace lexer {
enum class TokenType {
  END_OF_FILE,
  // Identifiers.
  LOWER_CASE_IDENT,
  MACRO_IDENT,
  UPPER_CASE_IDENT,
  // Literals.
  INT,
  HEX,
  OCT,
  BINARY_NUMBER,
  STRING,
  RAWSTRING,
  CODESTRING,
  RAWCODESTRING,
  // Binary operators.
  PLUS,
  MINUS,
  DIVIDE,
  STAR,
  OR,
  AND,
  GREATER,
  LESS,
  EQUAL,
  DIFFERENT,
  BITSHIFT_RIGHT,
  BITSHIFT_LEFT,
  BITOR,
  BITAND,
  BITXOR,
  ASSIGN,
  PLUS_ASSIGN,
  MINUS_ASSIGN,
  TIMES_ASSIGN,
  DIVIDE_ASSIGN,
  XOR_ASSIGN,
  OR_ASSIGN,
  AND_ASSIGN,
  // Unary operators.
  AMPERSAND,
  BANG,
  TILDE,
  // Delimiters.
  OPEN_PAREN,
  CLOSE_PAREN,
  OPEN_BRACKET,
  CLOSE_BRACKET,
  OPEN_BRACE,
  CLOSE_BRACE,
  SEMICOLON,
  COLON,
  COMMA,
  ARROW,  // ->
  // Keywords (alphabetical order).
  AS,
  DO,
  ENUM,
  DATA,
  CLASS,
  PRIVATE,
  PUBLIC,
  FROM,
  FOR,
  FORWARD,  // for forward-references.
  FUN,
  IF,
  IMPORT,
  IS,
  MUT,
  RETURN,
  VAL,
  WHEN,
  WHILE,
  // Mics.
  ANNOTATION,
  MACRO_CALL,
};

struct Location {
  int line;
  int column;
};

struct Token {
  const TokenType type;
  const std::string text;
  const Location begin;
  const Location end;
};

}  // namespace lexer
