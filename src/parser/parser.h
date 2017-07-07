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
  explicit ParseError(const std::string& message, const lexer::Range& location)
      : GenericError(location_line(message, location)) {}

  static std::string location_line(const std::string& message,
                                   const lexer::Range& location) {
    std::stringstream ss;
    ss << message << "\n At " << location.to_string();
    return ss.str();
  }
  ~ParseError() override = default;
};

// Parser class allows to parse any input.
class Parser {
 public:
  template <typename T>
  using ErrorOrPtr = ErrorOr<std::unique_ptr<T>>;
  // Initialize the parser with a Lexer.
  // The lexer is not owned by the Parser, it must be deleted.
  explicit Parser(lexer::Lexer* lexer);

  // Parse the input from the stream.
  ErrorOrPtr<ast::Module> parse();

 private:
  static constexpr unsigned int k_lookahead = 1;

  ErrorOrPtr<ast::ASTNode> parse_toplevel_declaration();
  ErrorOrPtr<ast::IntConstant> parse_int_constant();
  ErrorOrPtr<ast::VariableDeclaration> parse_variable_declaration();
  ErrorOrPtr<ast::Value> parse_value();
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
