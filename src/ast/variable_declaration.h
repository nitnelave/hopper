#pragma once

#include "ast/ast.h"
#include "ast/base_types.h"

namespace ast {

class VariableDeclaration : public ASTNode {
 public:
  VariableDeclaration(const lexer::Range& location, Identifier id, Type type,
                      std::unique_ptr<Value> value)
      : ASTNode(location),
        id_(std::move(id)),
        type_(std::move(type)),
        value_(std::move(value)) {}

  ~VariableDeclaration() override = default;

 private:
  void accept_impl(ASTVisitor& visitor) override { visitor.visit(this); }

  Identifier id_;
  Type type_;
  std::unique_ptr<Value> value_;
};

}  // namespace ast
