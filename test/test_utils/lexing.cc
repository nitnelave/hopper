#include "test_utils/lexing.h"

namespace lexer {

Range make_range(int line1, int col1, int line2, int col2,
                 const std::string& file) {
  return {file, line1, col1, line2, col2};
}

ErrorOr<std::vector<Token>> consume_tokens(Lexer& lexer) {
  std::vector<Token> result;
  while (true) {
    RETURN_OR_ASSIGN(Token tok, lexer.get_next_token());
    if (tok.type == TokenType::END_OF_FILE) return result;
    result.emplace_back(std::move(tok));
  }
  return result;
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
