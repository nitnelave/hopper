#pragma once

#include "ast/ast.h"

namespace ast {

class Value : public ASTNode {
 public:
  explicit Value(const lexer::Range& location) : ASTNode(location) {}
  ~Value() override = default;

 private:
  void accept_impl(ASTVisitor& visitor) override { visitor.visit(this); }
};

}  // namespace ast
