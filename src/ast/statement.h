#pragma once

#include "ast/ast.h"

namespace ast {

class Statement : public ASTNode {
 public:
  explicit Statement(lexer::Range location) : ASTNode(std::move(location)) {}
};

}  // namespace ast
