#include "parser/parser.h"

#include "ast/int_constant.h"
#include "ast/module.h"
#include "ast/variable_declaration.h"

#define EXPECT_TOKEN(TYPE, MESSAGE)   \
  if (current_token().type() != (TYPE)) \
    return ParseError((MESSAGE), current_token().location());

namespace parser {
using lexer::Token;
using lexer::TokenType;
using ast::Identifier;
using ast::Type;

Parser::Parser(Lexer* lexer) : lexer_(lexer) {}

ScopedLocation Parser::scoped_location() const { return ScopedLocation(this); }

ErrorOr<ast::Identifier> Parser::parse_type_identifier() {
  EXPECT_TOKEN(TokenType::UPPER_CASE_IDENT, "Expected type identifier");
  Identifier res(current_token().text());
  RETURN_IF_ERROR(get_token());
  return res;
}

ErrorOr<ast::Identifier> Parser::parse_value_identifier() {
  EXPECT_TOKEN(TokenType::LOWER_CASE_IDENT, "Expected value identifier");
  Identifier res(current_token().text());
  RETURN_IF_ERROR(get_token());
  return res;
}

ErrorOr<ast::Type> Parser::parse_type() {
  RETURN_OR_MOVE(Identifier id, parse_type_identifier());
  return Type(id);
}

Parser::ErrorOrPtr<ast::IntConstant> Parser::parse_int_constant() {
  auto location = scoped_location();
  auto value = current_token().int_value();
  RETURN_IF_ERROR(get_token());
  return std::make_unique<ast::IntConstant>(location.range(), value);
}

Parser::ErrorOrPtr<ast::Value> Parser::parse_value() {
  auto location = scoped_location();
  if (current_token().type() == TokenType::INT ||
      current_token().type() == TokenType::HEX ||
      current_token().type() == TokenType::OCT ||
      current_token().type() == TokenType::BINARY_NUMBER)
    return parse_int_constant();
  return ParseError("Expected value", location.error_range());
}

Parser::ErrorOrPtr<ast::VariableDeclaration>
Parser::parse_variable_declaration() {
  auto location = scoped_location();
  // Starts with VAL or MUT.
  assert(current_token().type() == TokenType::VAL ||
         current_token().type() == TokenType::MUT);
  bool mut = current_token().type() == TokenType::MUT;
  RETURN_IF_ERROR(get_token());
  // Then a name, lowercase.
  RETURN_OR_MOVE(Identifier variable_name, parse_value_identifier());
  // Then an optional type.
  Option<Type> type;
  if (current_token().type() == TokenType::COLON) {
    RETURN_IF_ERROR(get_token());
    RETURN_OR_MOVE(type, parse_type());
  }
  // Then an optional value.
  Option<std::unique_ptr<ast::Value>> value;
  if (current_token().type() == TokenType::ASSIGN) {
    RETURN_IF_ERROR(get_token());
    RETURN_OR_MOVE(value, parse_value());
  }
  // Then a semicolon.
  EXPECT_TOKEN(TokenType::SEMICOLON,
               "Expected semicolon at the end of the statement");
  RETURN_IF_ERROR(get_token());
  return std::make_unique<ast::VariableDeclaration>(
      location.range(), variable_name, std::move(type), std::move(value), mut);
}

Parser::ErrorOrPtr<ast::ASTNode> Parser::parse_toplevel_declaration() {
  auto location = scoped_location();
  if (current_token().type() == TokenType::VAL ||
      current_token().type() == TokenType::MUT) {
    return parse_variable_declaration();
  }
  return ParseError("Expected top-level declaration", location.error_range());
}

MaybeError<> Parser::get_token() {
  if (!token_stack_.empty()) {
    last_end_ = current_token().location().end;
  }
  do {
    RETURN_IF_ERROR(token_stack_.get_next());
  } while (current_token().type() == TokenType::COMMENT);
  return {};
}

void Parser::unget_token() { token_stack_.unget(); }

const Token& Parser::current_token() const { return token_stack_.current(); }

Parser::ErrorOrPtr<ast::Module> Parser::parse() {
  RETURN_IF_ERROR(get_token());
  auto location = scoped_location();
  std::vector<std::unique_ptr<ast::ASTNode>> declarations;
  while (current_token().type() != TokenType::END_OF_FILE) {
    RETURN_OR_MOVE(auto decl, parse_toplevel_declaration());
    declarations.emplace_back(std::move(decl));
  }
  return std::make_unique<ast::Module>(location.range(),
                                       std::move(declarations));
}

}  // namespace parser
