#pragma once

#include "ast/ast.h"

namespace ast {

class ValuedStatement : public Statement {
 public:
  explicit ValuedStatement(lexer::Range location, std::unique_ptr<Value> value)
      : Statement(std::move(location)), value_(std::move(value)) {}

  const std::unique_ptr<Value>& value() const { return value_; }

  ~ValuedStatement() override = default;

 private:
  void accept_impl(ASTVisitor& visitor) override { visitor.visit(this); }

  std::unique_ptr<Value> value_;
};

}  // namespace ast

