#include "parser/parser.h"

#include "ast/binary_operation.h"
#include "ast/block_statement.h"
#include "ast/boolean_constant.h"
#include "ast/function_call.h"
#include "ast/function_declaration.h"
#include "ast/if_statement.h"
#include "ast/int_constant.h"
#include "ast/local_variable_declaration.h"
#include "ast/module.h"
#include "ast/return_statement.h"
#include "ast/value_statement.h"
#include "ast/variable_declaration.h"
#include "ast/variable_reference.h"
#include "lexer/operators.h"

#define ASSERT_TOKEN(TYPE)                    \
  assert(current_token().type() == (TYPE) &&  \
         "This should be unreachable code."); \
  RETURN_IF_ERROR(get_token());

#define EXPECT_TOKEN(TYPE, MESSAGE)                       \
  if (current_token().type() != (TYPE))                   \
    return ParseError((MESSAGE), location.error_range()); \
  RETURN_IF_ERROR(get_token());

namespace parser {
using lexer::Token;
using lexer::TokenType;
using ast::Identifier;
using ast::Type;

Parser::Parser(Lexer* lexer) : lexer_(lexer) {}

ScopedLocation Parser::scoped_location() const { return ScopedLocation(this); }

ErrorOr<ast::Identifier> Parser::parse_type_identifier(IdentifierType type) {
  auto location = scoped_location();
  RETURN_OR_MOVE(Option<Identifier> id, parse_identifier(type));
  if (!id.is_ok() || !id.value_or_die().is_uppercase()) {
    return ParseError("Expected type identifier", location.error_range());
  }
  return id.value_or_die();
}

ErrorOr<ast::Identifier> Parser::parse_value_identifier(IdentifierType type) {
  auto location = scoped_location();
  RETURN_OR_MOVE(Option<Identifier> id, parse_identifier(type));

  if (!id.is_ok()) {
    return ParseError("Expected value identifier", location.error_range());
  }

  if (id.value_or_die().is_uppercase()) {
    return ParseError("Expected value identifier", location.range());
  }

  return id.value_or_die();
}

ErrorOr<Option<ast::Identifier>> Parser::parse_identifier(IdentifierType type) {
  auto location = scoped_location();
  bool absolute = false;
  std::stringstream text;
  text << current_token().text();
  if (current_token().type() == TokenType::COLON_COLON) {
    if (type == IdentifierType::SIMPLE)
      return ParseError("Unexpected '::', expected unqualified id",
                        location.error_range());
    absolute = true;
    RETURN_IF_ERROR(get_token());
    text << current_token().text();
  }
  while (current_token().type() == TokenType::UPPER_CASE_IDENT) {
    RETURN_IF_ERROR(get_token());
    if (type == IdentifierType::QUALIFIED &&
        current_token().type() == TokenType::COLON_COLON) {
      text << current_token().text();
      RETURN_IF_ERROR(get_token());
    } else {
      if (current_token().type() == TokenType::COLON_COLON)
        return ParseError("Unexpected '::', expected unqualified id",
                          location.error_range());
      return Identifier(text.str(), location.range(), true, absolute);
    }
  }
  if (current_token().type() == TokenType::LOWER_CASE_IDENT) {
    RETURN_IF_ERROR(get_token());
    if (current_token().type() == TokenType::COLON_COLON)
      return ParseError("Unexpected '::' after lowercase id",
                        location.error_range());
    return Identifier(text.str(), location.range(), false, absolute);
  }
  return none;
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

Parser::ErrorOrPtr<ast::Value> Parser::parse_value_no_operator() {
  auto location = scoped_location();

  if (current_token().type() == TokenType::OPEN_PAREN) {
    RETURN_IF_ERROR(get_token());
    RETURN_OR_MOVE(auto value, parse_value());
    EXPECT_TOKEN(TokenType::CLOSE_PAREN,
                 "Expected a ')' to match the opening one");
    return std::move(value);
  }

  if (current_token().type() == TokenType::TRUE ||
      current_token().type() == TokenType::FALSE) {
    bool bool_value = current_token().type() == TokenType::TRUE;
    RETURN_IF_ERROR(get_token());
    return std::make_unique<ast::BooleanConstant>(location.range(), bool_value);
  }

  if (current_token().type() == TokenType::INT ||
      current_token().type() == TokenType::HEX ||
      current_token().type() == TokenType::OCT ||
      current_token().type() == TokenType::BINARY_NUMBER) {
    return parse_int_constant();
  }

  if (current_token().type() == TokenType::LOWER_CASE_IDENT ||
      current_token().type() == TokenType::UPPER_CASE_IDENT ||
      current_token().type() == TokenType::COLON_COLON) {
    RETURN_OR_MOVE(Identifier id, parse_value_identifier());
    return std::make_unique<ast::VariableReference>(location.range(), id);
  }

  return ParseError("Expected value", location.error_range());
}

Parser::ErrorOrPtr<ast::Value> Parser::parse_value(int parent_precedence) {
  auto location = scoped_location();
  RETURN_OR_MOVE(auto value, parse_value_no_operator());

  // Parse function calls.
  while (current_token().type() == TokenType::OPEN_PAREN) {
    RETURN_IF_ERROR(get_token());
    std::vector<std::unique_ptr<ast::Value>> arguments;
    while (current_token().type() != TokenType::CLOSE_PAREN) {
      RETURN_OR_MOVE(auto arg, parse_value());
      arguments.emplace_back(std::move(arg));
      if (current_token().type() == TokenType::COMMA)
        RETURN_IF_ERROR(get_token());
      else
        break;
    }
    EXPECT_TOKEN(
        TokenType::CLOSE_PAREN,
        "Expected a closing parenthesis at the end of the function call");
    value = std::make_unique<ast::FunctionCall>(
        location.range(), std::move(value), std::move(arguments));
  }

  // Binary operator precedence resolution.
  // If the operator is of lower or equal precedence than the parent call,
  // delegate to the parent.
  // There is a double loop: the outer one corresponds to the precedence
  // resolution (each iteration handles a certain level of precedence,
  // delegating to a recursive call for higher precedence operators), and the
  // inner one corresponds to the left associativity (i.e. "2+3+4" is
  // "(2+3)+4"), staying at the same level of precedence.
  // "value" is always the left side of the operator, "binop" is always the
  // current (if valid) operator to handle, and "precedence" is the precedence
  // we are handling in the inner loop.
  Option<ast::BinaryOperator> binop =
      lexer::token_to_binary_operator(current_token().type());
  // Handle precedence.
  while (binop.is_ok() &&
         lexer::operator_precedence(binop.value_or_die()) > parent_precedence) {
    int precedence = lexer::operator_precedence(binop.value_or_die());
    // Handle associativity.
    while (binop.is_ok() &&
           lexer::operator_precedence(binop.value_or_die()) == precedence) {
      // Consume the operator.
      RETURN_IF_ERROR(get_token());
      // Parse the right side (up to the next operator).
      RETURN_OR_MOVE(auto right_value, parse_value(precedence));
      value = std::make_unique<ast::BinaryOp>(
          location.range(), std::move(value), binop.value_or_die(),
          std::move(right_value));
      // Peek at the next token.
      binop = lexer::token_to_binary_operator(current_token().type());
    }
  }

  return std::move(value);
}

template <class Declaration>
Parser::ErrorOrPtr<Declaration> Parser::parse_variable_declaration() {
  static_assert(std::is_base_of<ast::VariableDeclaration, Declaration>::value,
                "parse_variable_declaration is intended to parse "
                "VariableDeclaration classes");

  auto location = scoped_location();
  // Starts with VAL or MUT.
  assert(current_token().type() == TokenType::VAL ||
         current_token().type() == TokenType::MUT);
  bool mut = current_token().type() == TokenType::MUT;
  RETURN_IF_ERROR(get_token());
  // Then a simple name, lowercase.
  RETURN_OR_MOVE(Identifier variable_name,
                 parse_value_identifier(IdentifierType::SIMPLE));
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

  if (!value.is_ok() && !type.is_ok()) {
    return ParseError("Expected either a type or a default value to infer from",
                      location.error_range());
  }

  return std::make_unique<Declaration>(location.range(), variable_name,
                                       std::move(type), std::move(value), mut);
}

Parser::ErrorOrPtr<ast::BlockStatement> Parser::parse_statement_or_list() {
  auto location = scoped_location();

  std::unique_ptr<ast::BlockStatement> block;
  if (current_token().type() == TokenType::OPEN_BRACE) {
    RETURN_OR_MOVE(block, parse_statement_list());
  } else {
    RETURN_OR_MOVE(auto body, parse_statement());

    ast::BlockStatement::StatementList statement_list;
    statement_list.push_back(std::move(body));

    block = std::make_unique<ast::BlockStatement>(location.range(),
                                                  std::move(statement_list));
  }

  return std::move(block);
}

Parser::ErrorOrPtr<ast::IfStatement> Parser::parse_if_statement() {
  auto location = scoped_location();
  ASSERT_TOKEN(TokenType::IF);

  EXPECT_TOKEN(TokenType::OPEN_PAREN, "Expected '(' after 'if' keyword");
  RETURN_OR_MOVE(auto condition, parse_value());
  EXPECT_TOKEN(TokenType::CLOSE_PAREN, "Expected ')' before 'if' body");
  RETURN_OR_MOVE(auto if_body, parse_statement_or_list());

  if (current_token().type() == TokenType::ELSE) {
    RETURN_IF_ERROR(get_token());
    RETURN_OR_MOVE(auto else_statement, parse_statement_or_list());
    return std::make_unique<ast::IfStatement>(
        location.range(), std::move(condition), std::move(if_body),
        std::move(else_statement));
  }

  return std::make_unique<ast::IfStatement>(
      location.range(), std::move(condition), std::move(if_body), none);
}

Parser::ErrorOrPtr<ast::Statement> Parser::parse_statement() {
  auto location = scoped_location();

  if (current_token().type() == TokenType::OPEN_BRACE) {
    return parse_statement_list();
  }

  if (current_token().type() == TokenType::RETURN) {
    RETURN_IF_ERROR(get_token());
    if (current_token().type() == TokenType::SEMICOLON) {
      RETURN_IF_ERROR(get_token());
      return std::make_unique<ast::ReturnStatement>(location.range(), none);
    }

    // Otherwise, expect value.
    Option<std::unique_ptr<ast::Value>> value;
    RETURN_OR_MOVE(value, parse_value());
    EXPECT_TOKEN(TokenType::SEMICOLON,
                 "Expected `;' at the end of the statement");
    return std::make_unique<ast::ReturnStatement>(location.range(),
                                                  std::move(value));
  }

  if (current_token().type() == TokenType::IF) {
    return parse_if_statement();
  }

  if (current_token().type() == TokenType::ELSE) {
    return ParseError("A 'else' statement should follow a 'if' statement",
                      location.error_range());
  }

  if (current_token().type() == TokenType::VAL ||
      current_token().type() == TokenType::MUT) {
    RETURN_OR_MOVE(auto var_decl,
                   parse_variable_declaration<ast::LocalVariableDeclaration>());
    EXPECT_TOKEN(TokenType::SEMICOLON,
                 "Expected `;' at the end of a variable declaration");
    return std::move(var_decl);
  }

  if (current_token().type() == TokenType::FUN) {
    return parse_function_declaration();
  }

  auto value = parse_value();
  if (!value.is_ok()) {
    return ParseError("Could not parse as a statement", location.error_range());
  }
  EXPECT_TOKEN(TokenType::SEMICOLON,
               "Expected `;' at the end of the statement");
  return std::make_unique<ast::ValueStatement>(location.range(),
                                               std::move(value.value_or_die()));
}

Parser::ErrorOrPtr<ast::BlockStatement> Parser::parse_statement_list() {
  auto location = scoped_location();

  ASSERT_TOKEN(TokenType::OPEN_BRACE);

  ast::BlockStatement::StatementList statements;
  while (current_token().type() != TokenType::CLOSE_BRACE) {
    RETURN_OR_MOVE(auto sub_statement, parse_statement());

    statements.push_back(std::move(sub_statement));
  }

  EXPECT_TOKEN(TokenType::CLOSE_BRACE,
               "Expected `}' to match the opening brace");

  return std::make_unique<ast::BlockStatement>(location.range(),
                                               std::move(statements));
}

ErrorOr<ast::FunctionDeclaration::ArgumentList>
Parser::parse_function_arguments_declaration() {
  ast::FunctionDeclaration::ArgumentList arguments;

  if (current_token().type() == TokenType::CLOSE_PAREN) {
    return std::move(arguments);
  }

  bool should_parse_argument = true;
  while (should_parse_argument) {
    auto location = scoped_location();
    if (current_token().type() != TokenType::VAL &&
        current_token().type() != TokenType::MUT) {
      return ParseError("Expected function argument", location.error_range());
    }

    RETURN_OR_MOVE(
        auto val_decl,
        parse_variable_declaration<ast::FunctionArgumentDeclaration>());
    arguments.push_back(std::move(val_decl));

    if (current_token().type() == TokenType::COMMA) {
      RETURN_IF_ERROR(get_token());
    } else {
      should_parse_argument = false;
    }
  }

  return std::move(arguments);
}

Parser::ErrorOrPtr<ast::FunctionDeclaration>
Parser::parse_function_declaration() {
  auto location = scoped_location();
  EXPECT_TOKEN(TokenType::FUN, "Function declarations must start with `fun'");
  RETURN_OR_MOVE(Identifier fun_name,
                 parse_value_identifier(IdentifierType::SIMPLE));
  EXPECT_TOKEN(TokenType::OPEN_PAREN, "Expected `(' in function declaration");
  RETURN_OR_MOVE(auto arguments, parse_function_arguments_declaration());
  EXPECT_TOKEN(TokenType::CLOSE_PAREN, "Expected `)' after argument list");
  // Then an optional type.
  Option<Type> type;
  if (current_token().type() == TokenType::COLON) {
    RETURN_IF_ERROR(get_token());
    RETURN_OR_MOVE(type, parse_type());
  }

  auto body_location = scoped_location();
  if (current_token().type() == TokenType::OPEN_BRACE) {
    RETURN_OR_MOVE(auto body, parse_statement_list());
    return std::make_unique<ast::FunctionDeclaration>(
        location.range(), std::move(fun_name), std::move(arguments),
        std::move(type), std::move(body));
  }

  if (current_token().type() == TokenType::ASSIGN) {
    // fun my_fun() = 3;
    RETURN_IF_ERROR(get_token());
    RETURN_OR_MOVE(auto value, parse_value());
    EXPECT_TOKEN(TokenType::SEMICOLON,
                 "Missing ';' at the end of function declaration")

    return std::make_unique<ast::FunctionDeclaration>(
        location.range(), std::move(fun_name), std::move(arguments),
        std::move(type), std::move(value));
  }

  return ParseError("Expected function body", body_location.error_range());
}

Parser::ErrorOrPtr<ast::ASTNode> Parser::parse_toplevel_declaration() {
  auto location = scoped_location();
  if (current_token().type() == TokenType::VAL ||
      current_token().type() == TokenType::MUT) {
    RETURN_OR_MOVE(auto val_decl,
                   parse_variable_declaration<ast::LocalVariableDeclaration>());

    // Then a semicolon.
    EXPECT_TOKEN(TokenType::SEMICOLON,
                 "Expected `;' at the end of a variable declaration");

    return std::move(val_decl);
  }
  if (current_token().type() == TokenType::FUN) {
    return parse_function_declaration();
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
