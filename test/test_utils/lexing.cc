#include "test_utils/lexing.h"

namespace lexer {

ErrorOr<std::vector<Token>> consume_tokens(Lexer& lexer) {
  std::vector<Token> result;
  while (true) {
    RETURN_OR_MOVE(Token tok, lexer.get_next_token());
    if (tok.type() == TokenType::END_OF_FILE) return std::move(result);
    result.emplace_back(std::move(tok));
  }
  return std::move(result);
}

ErrorOr<std::vector<Token>> string_to_tokens(const std::string& input) {
  Lexer lex = lexer::from_string(input);
  return consume_tokens(lex);
}

ErrorOr<std::vector<Token>> file_to_tokens(const std::string& filename) {
  Lexer lex = lexer::from_file(filename);
  return consume_tokens(lex);
}
}  // namespace lexer
