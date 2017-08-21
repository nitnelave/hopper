#pragma once

#include "ast/ast.h"

namespace ast {

class Statement : public ASTNode {
 public:
  explicit Statement(lexer::Range location, NodeType node_type)
      : ASTNode(std::move(location), node_type) {}
  ~Statement() override = default;
};

}  // namespace ast
