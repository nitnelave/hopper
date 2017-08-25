#pragma once

#include <memory>
#include <vector>

#include "ast/base_types.h"
#include "ast/block_statement.h"
#include "ast/declaration.h"
#include "ast/function_argument_declaration.h"
#include "ast/value.h"
#include "util/option.h"
#include "visitor/visitor.h"

namespace ast {

class FunctionDeclaration : public Declaration {
 public:
  using ArgumentList =
      std::vector<std::unique_ptr<FunctionArgumentDeclaration>>;
  using ValueBody = std::unique_ptr<Value>;
  using StatementsBody = std::unique_ptr<BlockStatement>;

  /// Function declaration only.
  FunctionDeclaration(lexer::Range location, Identifier id,
                      ArgumentList arguments, Option<Type> type)
      : Declaration(std::move(location), NodeType::FUNCTION_DECLARATION,
                    std::move(id), std::move(type)),
        arguments_(std::move(arguments)),
        body_(none) {}

  /// Function declaration with a block statement.
  FunctionDeclaration(lexer::Range location, Identifier id,
                      ArgumentList arguments, Option<Type> type,
                      StatementsBody body)
      : Declaration(std::move(location), NodeType::FUNCTION_DECLARATION,
                    std::move(id), std::move(type)),
        arguments_(std::move(arguments)),
        body_(std::move(body)) {}

  /// Function declaration defined as a Value.
  FunctionDeclaration(lexer::Range location, Identifier id,
                      ArgumentList arguments, Option<Type> type, ValueBody body)
      : Declaration(std::move(location), NodeType::FUNCTION_DECLARATION,
                    std::move(id), std::move(type)),
        arguments_(std::move(arguments)),
        body_(std::move(body)) {}

  const std::string& name() { return id().to_string(); }

  const ArgumentList& arguments() const { return arguments_; }

  Variant<NoneType, StatementsBody, ValueBody>& body() { return body_; }

  ~FunctionDeclaration() override = default;

  void accept_body(ASTVisitor& visitor) {
    if (body_.is<StatementsBody>()) {
      body_.get_unchecked<StatementsBody>()->accept(visitor);
    } else {
      body_.get_unchecked<ValueBody>()->accept(visitor);
    }
  }

 private:
  void accept_impl(ASTVisitor& visitor) override { visitor.visit(this); }

  ArgumentList arguments_;
  Variant<NoneType, StatementsBody, ValueBody> body_;
};

}  // namespace ast
