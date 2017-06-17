#include "lexer/lexer.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace lexer {
namespace {
std::unique_ptr<std::istream> get_stream_from_source(const std::string& source,
                                                     Lexer::SourceTag tag) {
  switch (tag) {
    case Lexer::SourceTag::FILE:
      return std::make_unique<std::ifstream>(source);
    case Lexer::SourceTag::STRING:
      return std::make_unique<std::stringstream>(source);
    default:
      throw std::domain_error("Invalid tag when building the lexer");
  }
}

}  // namespace

Lexer::Lexer(const std::string& source, SourceTag tag)
    : stream_{get_stream_from_source(source, tag)} {}

Token Lexer::get_next_token() {
  return {TokenType::END_OF_FILE, "", location_, location_};
}

}  // namespace lexer
