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

  // Build a lexer object with the given source (raw string or filename).
  // Consider using the "from_*" family of functions.
  Lexer(const std::string& source, SourceTag tag,
        const std::string& filename = "");

  // Consume characters from the stream until a full token is seen, and return
  // that token, or a lexing error if a malformed token was seen.
  ErrorOr<Token, LexError> get_next_token();

  // Get the current location, in the source, of the lexer.
  const Location& get_location() const;

 private:
  // Consume a line comment (after the '//') and returns it.
  ErrorOr<Token, LexError> read_comment(const Location& beginning);
  // Consume a number in the given base and and returns a token of type tt with
  // the decimal value of the number in the text.
  ErrorOr<Token, LexError> read_base(const Location& beginning, TokenType tt,
                                     int base);

  // Consume a valid identifier, stopping at the first non-alphanumeric (or
  // '_') character, and returns a token of type tt (should be LOWER_CASE_IDENT
  // or UPPER_CASE_IDENT).
  ErrorOr<Token, LexError> read_identifier(TokenType tt);
  // Consume a valid lowercase identifier using read_identifier, and checks
  // against keywords. Returns either a LOWER_CASE_IDENT or the corresponding
  // keyword token.
  ErrorOr<Token, LexError> read_lowercase_identifier();
  // Puts the value of the next char into next_char_, and updates the location.
  void get_next_char();
  // Setup the lexer such that the next call to get_next_char() returns the
  // same character and location. The value of next_char_ is updated with the
  // previous value.
  void unget_char();

  char next_char_ = 0;
  char previous_char_ = 0;
  bool was_not_consumed_ = false;
  std::unique_ptr<std::istream> stream_;
  Location location_;
  Location previous_location_;
};

// Return an instance of Lexer that will read from the file.
Lexer from_file(const std::string& file);

// Return an instance of Lexer that will read from the given string.
// The filename is set to "<string>".
Lexer from_string(const std::string& text);

}  // namespace lexer
