#pragma once

#include <memory>

#include "ast/statement.h"
#include "ast/variable_declaration.h"
#include "visitor/visitor.h"

namespace ast {

class VariableDestruction : public Statement {
 public:
  explicit VariableDestruction(VariableDeclaration* declaration)
      : Statement(lexer::invalid_range(), NodeType::VARIABLE_DESTRUCTION),
        declaration_(declaration) {}

  VariableDeclaration* declaration() { return declaration_; }

  ~VariableDestruction() override = default;

 private:
  void accept_impl(ASTVisitor& visitor) override { visitor.visit(this); }

  VariableDeclaration* declaration_;
};

}  // namespace ast
