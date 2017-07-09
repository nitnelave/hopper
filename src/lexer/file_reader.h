#pragma once

#include <istream>
#include <memory>
#include <string>

#include "error/error.h"
#include "lexer/lex_error.h"
#include "lexer/token.h"

namespace lexer {

class FileReader {
 public:
  using State = std::pair<char, Location>;

  FileReader(std::unique_ptr<std::istream> stream, const std::string& filename)
      : stream_{std::move(stream)}, read_loc_{filename, 1, 0} {}

  ErrorOr<State, LexError> read_one_char();

 private:
  std::unique_ptr<std::istream> stream_;
  char read_char_ = 0;
  Location read_loc_;
};
}  // namespace lexer
