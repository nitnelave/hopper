#pragma once

#include <memory>

#include "ast/local_variable_declaration.h"
#include "util/option.h"
#include "visitor/visitor.h"

namespace ast {

class ExternVariableDeclaration : public LocalVariableDeclaration {
 public:
  ExternVariableDeclaration(lexer::Range location, Identifier id,
                            Option<Type> type,
                            Option<std::unique_ptr<Value>> value, bool mut)
      : LocalVariableDeclaration(std::move(location), std::move(id),
                                 std::move(type), std::move(value), mut) {}

  ~ExternVariableDeclaration() override = default;

 private:
  void accept_impl(ASTVisitor& visitor) override { visitor.visit(this); }
};

}  // namespace ast
