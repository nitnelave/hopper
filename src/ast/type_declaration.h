#pragma once

#include "ast/ast.h"
#include "ast/base_types.h"

namespace ast {
class TypeDeclaration : ASTNode {
 public:
  TypeDeclaration(lexer::Range location, NodeType node_type, Identifier id)
      : ASTNode(std::move(location), node_type), id_(std::move(id)) {}
  const Identifier& id() const { return id_; }

 private:
  Identifier id_;
};

}  // namespace ast
