#include "parser/parser.h"

namespace parser {
using lexer::Token;
using lexer::TokenType;

Parser::Parser(lexer::Lexer* lexer) : lexer_(lexer) {}

ParseError Parser::parse() {
  Token t = lexer_->get_next_token();
  if (t.type != TokenType::END_OF_FILE) return ParseErrorCode::VALID;
  return ParseErrorCode::SYNTAX_ERROR;
}

}  // namespace parser
