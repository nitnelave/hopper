#pragma once

#include <vector>

#include "ast/ast.h"
#include "ast/statement.h"

namespace ast {

class BlockStatement : public Statement {
 public:
  using StatementCollection = std::vector<std::unique_ptr<Statement>>;

  BlockStatement(lexer::Range location,
        StatementCollection statements)
      : Statement(std::move(location)), statements_(std::move(statements)) {}

  ~BlockStatement() override = default;

  StatementCollection const& statements() const { return statements_; }

 private:
  void accept_impl(ASTVisitor& visitor) override { visitor.visit(this); }

  StatementCollection statements_;
};

}  // namespace ast
