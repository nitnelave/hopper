#pragma once

#include <deque>
#include <memory>
#include <vector>

#include "ast/ast.h"
#include "ast/base_types.h"
#include "error/error.h"
#include "lexer/lexer.h"

namespace parser {

class ParseError : public GenericError {
 public:
  explicit ParseError(const std::string& message, const lexer::Token& t)
      : GenericError(location_line(message, t)) {}

  static std::string location_line(const std::string& message,
                                   const lexer::Token& t) {
    std::stringstream ss;
    ss << message << "\n At " << t.location.to_string();
    return ss.str();
  }
  ~ParseError() override = default;
};

// Parser class allows to parse any input.
class Parser {
 public:
  // Initialize the parser with a Lexer.
  // The lexer is not owned by the Parser, it must be deleted.
  explicit Parser(lexer::Lexer* lexer);

  // Parse the input from the stream.
  ErrorOr<ast::Module*> parse();

 private:
  static constexpr unsigned int k_lookahead = 1;

  ErrorOr<ast::ASTNode*> parse_toplevel_declaration();
  ErrorOr<ast::IntConstant*> parse_int_constant();
  ErrorOr<ast::VariableDeclaration*> parse_variable_declaration();
  ErrorOr<ast::Value*> parse_value();
  ErrorOr<ast::Type> parse_type();
  ErrorOr<ast::Identifier> parse_type_identifier();
  ErrorOr<ast::Identifier> parse_value_identifier();

  const lexer::Token& current_token() const;
  MaybeError<> get_token();
  void unget_token();

  lexer::Range range_from(const lexer::Range::Position& begin) const;

  // How many unget_token() levels we are at.
  unsigned int backlog_ = 0;
  std::deque<lexer::Token> token_stack_;
  lexer::Range::Position last_end_{0, 0};
  lexer::Lexer* lexer_;
};

}  // namespace parser
