#pragma once

#include <memory>
#include <vector>

#include "ast/ast.h"
#include "ast/base_types.h"
#include "ast/function_argument_declaration.h"
#include "ast/statement.h"
#include "util/option.h"

namespace ast {

class FunctionDeclaration : public ASTNode {
 public:
  FunctionDeclaration(
      lexer::Range location, Identifier id,
      std::vector<std::unique_ptr<FunctionArgumentDeclaration>> arguments,
      Option<Type> type, std::vector<std::unique_ptr<Statement>> body)
      : ASTNode(std::move(location)),
        id_(std::move(id)),
        arguments_(std::move(arguments)),
        type_(std::move(type)),
        body_(std::move(body)) {}

  FunctionDeclaration(
      lexer::Range location, Identifier id,
      std::vector<std::unique_ptr<FunctionArgumentDeclaration>> arguments,
      Option<Type> type, std::unique_ptr<Value> body)
      : ASTNode(std::move(location)),
        id_(std::move(id)),
        arguments_(std::move(arguments)),
        type_(std::move(type)),
        body_(std::move(body)) {}

  const Identifier& id() const { return id_; }

  const Option<Type>& type() const { return type_; }

  const std::vector<std::unique_ptr<FunctionArgumentDeclaration>>& arguments()
      const {
    return arguments_;
  }

  const Variant<std::vector<std::unique_ptr<Statement>>,
                std::unique_ptr<Value>>&
  body() const {
    return body_;
  }

  ~FunctionDeclaration() override = default;

 private:
  void accept_impl(ASTVisitor& visitor) override { visitor.visit(this); }

  Identifier id_;
  std::vector<std::unique_ptr<FunctionArgumentDeclaration>> arguments_;
  Option<Type> type_;
  Variant<std::vector<std::unique_ptr<Statement>>, std::unique_ptr<Value>>
      body_;
};

}  // namespace ast
