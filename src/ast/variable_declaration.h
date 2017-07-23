#pragma once

#include "ast/ast.h"
#include "ast/base_types.h"
#include "util/option.h"
#include "visitor/visitor.h"

namespace ast {

class VariableDeclaration : public ASTNode {
 public:
  VariableDeclaration(lexer::Range location, Identifier id, Option<Type> type,
                      Option<std::unique_ptr<Value>> value, bool mut)
      : ASTNode(std::move(location)),
        id_(std::move(id)),
        type_(std::move(type)),
        value_(std::move(value)),
        mut_(mut) {}

  bool is_mutable() const { return mut_; }

  const Identifier& id() const { return id_; }

  const Option<Type>& type() const { return type_; }

  const Option<std::unique_ptr<Value>>& value() const { return value_; }

  ~VariableDeclaration() override = default;

 private:
  void accept_impl(ASTVisitor& visitor) override { visitor.visit(this); }

  Identifier id_;
  Option<Type> type_;
  Option<std::unique_ptr<Value>> value_;
  bool mut_;
};

}  // namespace ast
