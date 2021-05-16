#pragma once

#include "ast/statement.h"
#include "visitor/visitor.h"

namespace ast {

class UnreachableStatement : public Statement {
 public:
  explicit UnreachableStatement(lexer::Range range)
      : Statement(std::move(range), NodeType::UNREACHABLE) {}

  bool value() const { return value_; }

 private:
  void accept_impl(ASTVisitor& visitor) override { visitor.visit(this); }
  bool value_;
};
}  // namespace ast
