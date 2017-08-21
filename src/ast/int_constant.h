#pragma once

#include "ast/value.h"
#include "visitor/visitor.h"

namespace ast {

class IntConstant : public Value {
 public:
  IntConstant(lexer::Range location, int64_t value)
      : Value(std::move(location), NodeType::INT_CONSTANT), value_(value) {}
  ~IntConstant() override = default;

  int64_t value() const { return value_; }

 private:
  void accept_impl(ASTVisitor& visitor) override { visitor.visit(this); }
  int64_t value_;
};

}  // namespace ast
