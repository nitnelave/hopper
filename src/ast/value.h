#pragma once

#include "ast/ast.h"
#include "ast/base_types.h"
#include "util/option.h"

namespace ast {

enum ValueType {
  UNKNOWN = 0,
  CONSTANT = 1,
  VARIABLE = 2,
  RESULT = 3,
};

class Value : public ASTNode {
 public:
  explicit Value(lexer::Range location, ValueType value_type)
      : ASTNode(std::move(location)), value_type_(value_type) {
    assert(value_type != UNKNOWN && "A variable type must not be unknown");
  }
  ~Value() override = default;

  Option<Type>& type() { return type_; }
  ValueType value_type() { return value_type_; }

 private:
  Option<Type> type_ = none;
  ValueType value_type_ = UNKNOWN;
};

}  // namespace ast
