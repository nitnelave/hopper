#include "parser/parser.h"

namespace parser {
using lexer::Token;
using lexer::TokenType;

Parser::Parser(lexer::Lexer* lexer) : lexer_(lexer) {}

MaybeError<GenericError> Parser::parse() {
  RETURN_OR_ASSIGN(const Token& t, lexer_->get_next_token());
  if (t.type != TokenType::END_OF_FILE) return {};
  return ParseError{"Unexpected EOF", t};
}

}  // namespace parser
