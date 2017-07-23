#pragma once

#include "ast/ast.h"
#include "ast/base_types.h"
#include "ast/value.h"
#include "visitor/visitor.h"

namespace ast {

class VariableReference : public Value {
 public:
  VariableReference(lexer::Range location, Identifier id)
      : Value(std::move(location)), id_(std::move(id)) {}

  const Identifier& id() const { return id_; }

  ~VariableReference() override = default;

 private:
  void accept_impl(ASTVisitor& visitor) override { visitor.visit(this); }

  Identifier id_;
};

}  // namespace ast
