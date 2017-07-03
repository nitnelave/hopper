#pragma once

#include <istream>
#include <memory>
#include <string>

#include "error/error.h"
#include "lexer/token.h"

namespace lexer {
class LexError : public GenericError {
 public:
  explicit LexError(const std::string& message, Location l)
      : GenericError(message + "at " + l.to_string()) {}
  explicit LexError(const std::string& message, Location begin, Location end)
      : GenericError(message + " in " + begin.file + " from " +
                     std::to_string(begin.line) + ":" +
                     std::to_string(begin.column) + " to " +
                     std::to_string(end.line) + ":" +
                     std::to_string(end.column)) {}
};

class Lexer {
 public:
  // What type of input is given to the constructor.
  enum class SourceTag {
    FILE,
    STRING,
  };

  Lexer(const std::string& source, SourceTag tag,
        const std::string& filename = "");

  ErrorOr<Token, LexError> get_next_token();

  const Location& get_location() const;

 private:
  ErrorOr<Token, LexError> read_comment(const Location& beginning);
  ErrorOr<Token, LexError> read_base(const Location& beginning, TokenType tt,
                                     int base);
  // Puts the value of the next char into next_char_, and updates the location.
  void get_next_char();
  void unget_char();

  char next_char_ = 0;
  char previous_char_ = 0;
  bool was_not_consumed_ = false;
  std::unique_ptr<std::istream> stream_;
  Location location_;
  Location previous_location_;
};

Lexer from_file(const std::string& file);
Lexer from_string(const std::string& text);

}  // namespace lexer
