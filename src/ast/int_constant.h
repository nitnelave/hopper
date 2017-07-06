#pragma once

#include "ast/value.h"

namespace ast {

class IntConstant : public Value {
 public:
  IntConstant(const lexer::Range& location, int64_t value)
      : Value(location), value_(value) {}
  ~IntConstant() override = default;

  int64_t value() const { return value_; }

 private:
  void accept_impl(ASTVisitor& visitor) override { visitor.visit(this); }
  int64_t value_;
};

}  // namespace ast
