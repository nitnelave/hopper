#pragma once

#include "ast/ast.h"
#include "ast/base_types.h"
#include "visitor/visitor.h"

namespace ast {

class FunctionArgumentDeclaration : public ASTNode {
 public:
  FunctionArgumentDeclaration(lexer::Range location, Identifier id, Type type)
      : ASTNode(std::move(location)),
        id_(std::move(id)),
        type_(std::move(type)) {}

  const Identifier& id() const { return id_; }

  const Type& type() const { return type_; }

 private:
  Identifier id_;
  Type type_;
};

}  // namespace ast
