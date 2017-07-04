#pragma once

#include "error/error.h"
#include "lexer/lexer.h"

namespace parser {

class ParseError : public GenericError {
 public:
  explicit ParseError(const std::string& message, const lexer::Token& t)
      : GenericError(location_line(message, t)) {}

  static std::string location_line(const std::string& message,
                                   const lexer::Token& t) {
    std::stringstream ss;
    ss << message << "\n At " << t.location.to_string();
    return ss.str();
  }
};

// Parser class allows to parse any input.
class Parser {
 public:
  // Initialize the parser with a Lexer.
  // The lexer is not owned by the Parser, it must be deleted.
  explicit Parser(lexer::Lexer* lexer);

  // Parse the input from the stream.
  MaybeError<GenericError> parse();

 private:
  lexer::Lexer* lexer_;
};

}  // namespace parser
