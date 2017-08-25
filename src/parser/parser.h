#pragma once

#include <deque>
#include <memory>
#include <vector>

#include "ast/ast.h"
#include "ast/base_types.h"
#include "ast/block_statement.h"
#include "ast/function_declaration.h"
#include "ast/variable_declaration.h"
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

  /// TopLevel:
  /// <VariableDeclaration>|<FunctionDeclaration>
  ErrorOrPtr<ast::ASTNode> parse_toplevel_declaration();

  /// IntConstant:
  /// <intValue>|<hexValue>|<octValue>|<binValue>
  ErrorOrPtr<ast::IntConstant> parse_int_constant();

  /// VariableDeclaration:
  /// (val|mut) <variable_name> [: <type>] [= <value>];
  template <class Declaration>
  ErrorOrPtr<Declaration> parse_variable_declaration(
      ast::CallingConvention calling_convention);

  /// FunctionDeclaration:
  /// fun <ValueId> (<FuncArgsDecl>) [: <Type>] (= <Value>;|<BlockStatement>)
  ErrorOrPtr<ast::FunctionDeclaration> parse_function_declaration(
      ast::CallingConvention calling_convention);

  /// FuncArgsDecl:
  /// [<Identifier>: <Type>][, <Identifier>: <Type>]...
  ErrorOr<ast::FunctionDeclaration::ArgumentList>
  parse_function_arguments_declaration();

  ErrorOrPtr<ast::Declaration> parse_extern_declaration();

  ErrorOr<ast::CallingConvention> parse_calling_convention();

  /// Value:
  /// <ValueNoOp> [([<Value>[COMMA <Value>]...])]... [<Operator> <Value> ]...
  ErrorOrPtr<ast::Value> parse_value(int parent_precedence = 0);

  /// ValueNoOp:
  /// [(<Value>)|true|false|<IntConstant>|<ValueId>]
  ErrorOrPtr<ast::Value> parse_value_no_operator();

  /// Statement list:
  /// { <Statement> ... }
  ErrorOrPtr<ast::BlockStatement> parse_statement_list();

  /// IfStmt:
  /// if ( <Value> ) [else (<Statement|IfStmt)]
  ErrorOrPtr<ast::IfStatement> parse_if_statement();

  /// Statement:
  /// value;
  /// return [value];
  ErrorOrPtr<ast::Statement> parse_statement();

  /// Type:
  /// <TypeIdentifier>
  ErrorOr<ast::Type> parse_type();

  enum class IdentifierType {
    // Unqualified identifier, such the name of a variable in a declaration.
    // e.g. "foobar"
    SIMPLE,
    // Qualified identifier, with potential module/class prefix.
    // e.g. "Module::foobar"
    QUALIFIED,
  };

  /// TypeIndentifier:
  /// <UpperCaseIdentifier>
  ErrorOr<ast::Identifier> parse_type_identifier(
      IdentifierType type = IdentifierType::QUALIFIED);

  /// ValueId:
  /// <LowerCaseIdentifier>
  ErrorOr<ast::Identifier> parse_value_identifier(
      IdentifierType type = IdentifierType::QUALIFIED);

  /// Identifier:
  /// (::)?[<Upper>::]*(<Upper>|<lower>)
  ErrorOr<Option<ast::Identifier>> parse_identifier(
      IdentifierType type = IdentifierType::QUALIFIED);

  /// Calls parse_statement_list if an open brace is detected or parse_statement
  /// otherwise.
  /// In the case of the simple statement, a BlockStatement will be built out of
  /// it.
  ErrorOrPtr<ast::BlockStatement> parse_statement_or_list();

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
