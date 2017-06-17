#pragma once

#include "lexer/lexer.h"

namespace parser {
// Different error types that can be returned from parsing.
enum class ParseErrorCode {
  VALID,
  SYNTAX_ERROR,
};

// Error produced while parsing (TODO: make it more than an enum)
using ParseError = ParseErrorCode;

// Parser class allows to parse any input.
class Parser {
 public:
  // Initialize the parser with a Lexer.
  // The lexer is not owned by the Parser, it must be deleted.
  explicit Parser(lexer::Lexer* lexer);

  // Parse the input from the stream.
  ParseError parse();

 private:
  lexer::Lexer* lexer_;
};

}  // namespace parser
