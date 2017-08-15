#include <memory>

#include "ast/ast.h"
#include "ast/value.h"
#include "lexer/operators.h"
#include "visitor/visitor.h"

namespace ast {

using BinaryOperator = lexer::BinaryOperator;

class BinaryOp : public Value {
 public:
  BinaryOp(lexer::Range location, std::unique_ptr<Value> left,
           BinaryOperator op, std::unique_ptr<Value> right)
      : Value(std::move(location), ValueType::RESULT),
        left_(std::move(left)),
        op_(op),
        right_(std::move(right)) {}

  Value& left_value() { return *left_; }

  BinaryOperator operation() { return op_; }

  Value& right_value() { return *right_; }

  ~BinaryOp() override = default;

 private:
  void accept_impl(ASTVisitor& visitor) override { visitor.visit(this); }
  std::unique_ptr<Value> left_;
  BinaryOperator op_;
  std::unique_ptr<Value> right_;
};
}  // namespace ast
