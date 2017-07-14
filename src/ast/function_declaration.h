#pragma once

#include <memory>
#include <vector>

#include "ast/ast.h"
#include "ast/base_types.h"
#include "ast/function_argument_declaration.h"
#include "ast/statement.h"
#include "ast/value.h"
#include "util/option.h"

namespace ast {

class FunctionDeclaration : public ASTNode {
 public:
  using ArgumentList =
      std::vector<std::unique_ptr<FunctionArgumentDeclaration>>;
  using ValueBody = std::unique_ptr<Value>;
  using StatementsBody = std::vector<std::unique_ptr<Statement>>;
  FunctionDeclaration(lexer::Range location, Identifier id,
                      ArgumentList arguments, Option<Type> type,
                      StatementsBody body)
      : ASTNode(std::move(location)),
        id_(std::move(id)),
        arguments_(std::move(arguments)),
        type_(std::move(type)),
        body_(std::move(body)) {}

  FunctionDeclaration(lexer::Range location, Identifier id,
                      ArgumentList arguments, Option<Type> type, ValueBody body)
      : ASTNode(std::move(location)),
        id_(std::move(id)),
        arguments_(std::move(arguments)),
        type_(std::move(type)),
        body_(std::move(body)) {}

  const Identifier& id() const { return id_; }

  const Option<Type>& type() const { return type_; }

  const ArgumentList& arguments() const { return arguments_; }

  Variant<StatementsBody, ValueBody>& body() { return body_; }

  ~FunctionDeclaration() override = default;

  void accept_body(ASTVisitor& visitor) {
    if (body_.is<StatementsBody>()) {
      for (const auto& statement : body_.get_unchecked<StatementsBody>())
        statement->accept(visitor);
    } else {
      body_.get_unchecked<ValueBody>()->accept(visitor);
    }
  }

 private:
  void accept_impl(ASTVisitor& visitor) override { visitor.visit(this); }

  Identifier id_;
  ArgumentList arguments_;
  Option<Type> type_;
  Variant<StatementsBody, ValueBody> body_;
};

}  // namespace ast
