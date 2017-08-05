#pragma once

#include <memory>

#include "ast/variable_declaration.h"
#include "util/option.h"
#include "visitor/visitor.h"

namespace ast {

class LocalVariableDeclaration : public VariableDeclaration {
 public:
  LocalVariableDeclaration(lexer::Range location, Identifier id,
                           Option<Type> type,
                           Option<std::unique_ptr<Value>> value, bool mut)
      : VariableDeclaration(std::move(location), std::move(id), std::move(type),
                            std::move(value), mut) {}

  ~LocalVariableDeclaration() override = default;

 private:
  void accept_impl(ASTVisitor& visitor) override { visitor.visit(this); }
};

}  // namespace ast
