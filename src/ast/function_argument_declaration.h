#pragma once

#include "ast/ast.h"
#include "ast/base_types.h"
#include "visitor/visitor.h"

namespace ast {

class FunctionArgumentDeclaration : public ASTNode {
 public:
  FunctionArgumentDeclaration(lexer::Range location, Identifier id, Type type,
                              bool mut)
      : ASTNode(std::move(location)),
        id_(std::move(id)),
        type_(std::move(type)),
        mut_(mut) {}

  bool is_mutable() const { return mut_; }

  const Identifier& id() const { return id_; }

  const Type& type() const { return type_; }

 private:
  void accept_impl(ASTVisitor& visitor) override { visitor.visit(this); }

  Identifier id_;
  Type type_;
  bool mut_;
};

}  // namespace ast
