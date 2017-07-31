#pragma once

#include "ast/ast.h"

namespace ast {

class Value : public ASTNode {
 public:
  explicit Value(lexer::Range location) : ASTNode(std::move(location)) {}
  ~Value() override = default;
};

}  // namespace ast
