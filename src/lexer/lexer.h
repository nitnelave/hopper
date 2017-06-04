#pragma once

#include <istream>
#include <memory>
#include <string>

#include "lexer/token.h"

namespace lexer {
class Lexer {
 public:
  // What type of input is given to the constructor.
  enum class SourceTag {
    FILE,
    STRING,
  };
  Lexer(const std::string& source, SourceTag tag);

  static Lexer from_file(const std::string& file) {
    return Lexer(file, SourceTag::FILE);
  }

  static Lexer from_string(const std::string& text) {
    return Lexer(text, SourceTag::STRING);
  }

  Token get_next_token();

 private:
  std::unique_ptr<std::istream> stream_;
  Location location_{1, 0};
};

}  // namespace lexer
