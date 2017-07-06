#include "parser/parser.h"

#include "ast/int_constant.h"
#include "ast/variable_declaration.h"

namespace parser {
using lexer::Token;
using lexer::TokenType;
using ast::Identifier;
using ast::Type;

Parser::Parser(lexer::Lexer* lexer) : lexer_(lexer) {}

auto Parser::parse_variable_declaration()
    -> ErrorOr<ast::VariableDeclaration*> {
  return nullptr;
}

auto Parser::parse_toplevel_declaration() -> ErrorOr<ast::ASTNode*> {
  const Token& t = current_token();
  if (t.type == TokenType::VAL || t.type == TokenType::MUT) {
    return parse_variable_declaration();
  }
  return nullptr;
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
  RETURN_IF_ERROR(lexer_->get_next_token());
  std::vector<std::unique_ptr<ast::ASTNode>> declarations;
  while (current_token().type != TokenType::END_OF_FILE) {
    RETURN_OR_ASSIGN(auto decl, parse_toplevel_declaration());
    declarations.emplace_back(decl);
  }
  return std::move(declarations);
}

}  // namespace parser
