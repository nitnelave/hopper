#pragma once

#include <memory>
#include <vector>

#include "ast/ast.h"
#include "ast/statement.h"
#include "visitor/visitor.h"

namespace ast {

class BlockStatement : public Statement {
 public:
  using StatementList = std::vector<std::unique_ptr<Statement>>;

  BlockStatement(lexer::Range location, StatementList statements)
      : Statement(std::move(location)), statements_(std::move(statements)) {}

  ~BlockStatement() override = default;

  StatementList const& statements() const { return statements_; }

 private:
  void accept_impl(ASTVisitor& visitor) override { visitor.visit(this); }

  StatementList statements_;
};

}  // namespace ast
