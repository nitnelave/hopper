#pragma once

#include <istream>
#include <memory>
#include <string>

#include "error/error.h"
#include "lexer/file_reader.h"
#include "lexer/lex_error.h"
#include "lexer/token.h"
#include "util/lookahead_stack.h"

namespace lexer {
namespace internals {
class LexerHelper;
}  // namespace internals
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
  const Location& location() const;

 private:
  // Consume a line comment (after the '//') and returns it.
  ErrorOr<Token, LexError> read_comment(const Location& beginning);
  // Consume a number in the given base and and returns a token of type tt with
  // the decimal value of the number in the text.
  ErrorOr<Token, LexError> read_base(const Location& beginning, TokenType tt,
                                     int base);

  // Consume characters until a valid double quote is met.
  ErrorOr<Token, LexError> read_string(const Location& beginning);

  // Consume a valid identifier, stopping at the first non-alphanumeric (or
  // '_') character, and returns a token of type tt (should be LOWER_CASE_IDENT
  // or UPPER_CASE_IDENT).
  ErrorOr<Token, LexError> read_identifier(TokenType tt);
  // Consume a valid lowercase identifier using read_identifier, and checks
  // against keywords. Returns either a LOWER_CASE_IDENT or the corresponding
  // keyword token.
  ErrorOr<Token, LexError> read_lowercase_identifier();
  // Reads the next char from the stream, updating the location. May return an
  // error if read operation fails.
  MaybeError<LexError> get_next_char();
  // Push the current character and location on a stack, and restore the
  // previous one.
  void unget_char();

  // Returns the current char to be examined.
  char current_char() const;

  // The amount of lookahead needed for lexing (2 char needed to lex "?->").
  static constexpr int k_lookahead = 2;

  // Holds the stream and the current state of the stream. To be passed to the
  // LookaheadStack.
  FileReader reader_;

  // Stack that will take care of getting/ungetting chars, by calling the
  // callback to get new chars, at the appropriate moments.
  using CharStack =
      util::LookaheadStack</*maximum lookahead needed*/ k_lookahead,
                           /*values in the stack*/ FileReader::State,
                           /*potential error type*/ LexError>;
  CharStack char_stack_ =
      CharStack(std::bind(&FileReader::read_one_char, &reader_));
  friend class internals::LexerHelper;
};

/// Return an instance of Lexer that will read from the file.
Lexer from_file(const std::string& file);

/// Return an instance of Lexer that will read from the given string.
/// The filename is set to "<string>".
Lexer from_string(const std::string& text);

}  // namespace lexer
