#pragma once

#include <deque>
#include <memory>
#include <vector>

#include "ast/ast.h"
#include "ast/base_types.h"
#include "error/error.h"
#include "lexer/lexer.h"
#include "parser/scoped_location.h"
#include "util/lookahead_stack.h"

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
  using Lexer = lexer::Lexer;
  using Range = lexer::Range;

  template <typename T>
  using ErrorOrPtr = ErrorOr<std::unique_ptr<T>>;
  // Initialize the parser with a Lexer.
  // The lexer is not owned by the Parser, it must be deleted.
  explicit Parser(Lexer* lexer);

  // Parse the input from the stream.
  ErrorOrPtr<ast::Module> parse();

 private:
  static constexpr unsigned int k_lookahead = 0;

  ErrorOrPtr<ast::ASTNode> parse_toplevel_declaration();
  ErrorOrPtr<ast::IntConstant> parse_int_constant();
  ErrorOrPtr<ast::VariableDeclaration> parse_variable_declaration();
  ErrorOrPtr<ast::Value> parse_value();
  ErrorOr<std::vector<std::unique_ptr<ast::Statement>>> parse_statement_list();
  ErrorOrPtr<ast::Statement> parse_statement();
  ErrorOr<ast::Type> parse_type();
  ErrorOr<ast::Identifier> parse_type_identifier(bool simple = false);
  ErrorOr<ast::Identifier> parse_value_identifier(bool simple = false);
  ErrorOr<Option<ast::Identifier>> parse_identifier(bool simple = false);

  const lexer::Token& current_token() const;
  MaybeError<> get_token();
  void unget_token();

  ScopedLocation scoped_location() const;

  Range::Position last_end_{0, 0};
  Lexer* lexer_;
  using TokenStack =
      util::LookaheadStack<k_lookahead, lexer::Token, lexer::LexError>;
  TokenStack token_stack_ =
      TokenStack{std::bind(&Lexer::get_next_token, lexer_)};
  friend class ScopedLocation;
};

}  // namespace parser
