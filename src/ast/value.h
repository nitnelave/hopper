#pragma once

#include "ast/ast.h"
#include "ast/base_types.h"
#include "util/option.h"

namespace ast {

class Value : public ASTNode {
 public:
  explicit Value(lexer::Range location, NodeType node_type)
      : ASTNode(std::move(location), node_type) {}
  ~Value() override = default;

  Option<Type>& type() { return type_; }

 private:
  Option<Type> type_ = none;
};

}  // namespace ast
