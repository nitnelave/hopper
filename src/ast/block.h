#pragma once

#include "ast/ast.h"
#include "ast/statement.h"
#include "ast/value.h"

namespace ast {

class Block : public Statement {
 public:
  Block(lexer::Range location,
        std::vector<std::unique_ptr<Statement>> statements)
      : Statement(std::move(location)), statements_(std::move(statements)) {}

  ~Block() override = default;

  std::vector<std::unique_ptr<Statement>>& statements() { return statements_; }

 private:
  void accept_impl(ASTVisitor& visitor) override { visitor.visit(this); }

  std::vector<std::unique_ptr<Statement>> statements_;
};

}  // namespace ast
