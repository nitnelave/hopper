#pragma once

#include "ast/ast.h"

namespace ast {

class Value : public ASTNode {
 public:
  explicit Value(lexer::Range location) : ASTNode(std::move(location)) {}
  ~Value() override = default;

 private:
  void accept_impl(ASTVisitor& visitor) override { visitor.visit(this); }
};

}  // namespace ast
