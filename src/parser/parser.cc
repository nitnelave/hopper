#include "parser/parser.h"

#include "ast/int_constant.h"
#include "ast/variable_declaration.h"

#define EXPECT_TOKEN(TYPE, MESSAGE) \
  if (current_token().type != TYPE) \
    return ParseError(MESSAGE, current_token());

namespace parser {
using lexer::Token;
using lexer::TokenType;
using ast::Identifier;
using ast::Type;

Parser::Parser(lexer::Lexer* lexer) : lexer_(lexer) {}

ErrorOr<ast::Identifier> Parser::parse_type_identifier() {
  EXPECT_TOKEN(TokenType::UPPER_CASE_IDENT, "Expected type identifier");
  Identifier res(current_token().text);
  RETURN_IF_ERROR(get_token());
  return res;
}

ErrorOr<ast::Identifier> Parser::parse_value_identifier() {
  EXPECT_TOKEN(TokenType::LOWER_CASE_IDENT, "Expected value identifier");
  Identifier res(current_token().text);
  RETURN_IF_ERROR(get_token());
  return res;
}

ErrorOr<ast::Type> Parser::parse_type() {
  RETURN_OR_ASSIGN(Identifier id, parse_type_identifier());
  return Type(id);
}

ErrorOr<ast::IntConstant*> Parser::parse_int_constant() {
  auto location = current_token().location;
  auto value = std::atoi(current_token().text.c_str());
  RETURN_IF_ERROR(get_token());
  return new ast::IntConstant(location, value);
}

ErrorOr<ast::Value*>  Parser::parse_value() {
  if (current_token().type == TokenType::INT ||
      current_token().type == TokenType::HEX ||
      current_token().type == TokenType::OCT ||
      current_token().type == TokenType::BINARY_NUMBER)
    return parse_int_constant();
  return ParseError("Expected value", current_token());
}

ErrorOr<ast::VariableDeclaration*>  Parser::parse_variable_declaration()
    {
  // Starts with VAL or MUT.
  assert(current_token().type == TokenType::VAL || current_token().type == TokenType::MUT);
  auto begin = current_token().location.begin;
  bool mut = current_token().type == TokenType::MUT;
  RETURN_IF_ERROR(get_token());
  // Then a name, lowercase.
  RETURN_OR_ASSIGN(Identifier variable_name, parse_value_identifier());
  // Then an optional type.
  Option<Type> type;
  if (current_token().type == TokenType::COLON) {
    RETURN_IF_ERROR(get_token());
    RETURN_OR_ASSIGN(type, parse_type());
  }
  // Then an optional value.
  Option<std::unique_ptr<ast::Value>> value;
  if (current_token().type == TokenType::ASSIGN) {
    RETURN_IF_ERROR(get_token());
    RETURN_OR_ASSIGN(value, parse_value());
  }
  // Then a semicolon.
  EXPECT_TOKEN(TokenType::SEMICOLON, "Expected semicolon at the end of the statement");
  auto end = current_token().location.end;
  RETURN_IF_ERROR(get_token());
  return new ast::VariableDeclaration({current_token().location.file, begin, end}, variable_name, std::move(type), std::move(value), mut);
}

ErrorOr<ast::ASTNode*> Parser::parse_toplevel_declaration() {
  const Token& t = current_token();
  if (t.type == TokenType::VAL || t.type == TokenType::MUT) {
    return parse_variable_declaration();
  }
  return ParseError("Expected top-level declaration", current_token());
}

MaybeError<> Parser::get_token() {
  if (token_stack_.size() > k_lookahead) {
    token_stack_.pop_front();
  }
  if (backlog_ == 0) {
    RETURN_OR_ASSIGN(const Token& t, lexer_->get_next_token());
    token_stack_.push_back(t);
  } else {
    --backlog_;
  }
  return {};
}

void Parser::unget_token() {
  ++backlog_;
  assert(backlog_ <= k_lookahead &&
         "Too much token backlog; increase k_lookahead?");
}

const Token& Parser::current_token() const {
  assert(token_stack_.size() > backlog_);
  return token_stack_[token_stack_.size() - backlog_ - 1];
}

ErrorOr<std::vector<std::unique_ptr<ast::ASTNode>>> Parser::parse() {
  RETURN_IF_ERROR(get_token());
  std::vector<std::unique_ptr<ast::ASTNode>> declarations;
  while (current_token().type != TokenType::END_OF_FILE) {
    RETURN_OR_ASSIGN(auto decl, parse_toplevel_declaration());
    declarations.emplace_back(decl);
  }
  return std::move(declarations);
}

}  // namespace parser
