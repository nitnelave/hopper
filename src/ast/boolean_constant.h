#pragma once

#include "ast/value.h"
#include "visitor/visitor.h"

namespace ast {

class BooleanConstant : public Value {
 public:
  BooleanConstant(lexer::Range range, bool value)
      : Value(std::move(range), NodeType::BOOLEAN_CONSTANT), value_(value) {}

  bool value() const { return value_; }

 private:
  void accept_impl(ASTVisitor& visitor) override { visitor.visit(this); }
  bool value_;
};
}  // namespace ast
