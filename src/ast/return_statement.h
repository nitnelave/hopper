#pragma once

#include "ast/ast.h"
#include "ast/base_types.h"
#include "ast/statement.h"
#include "util/option.h"

namespace ast {

class ReturnStatement : public Statement {
 public:
  ReturnStatement(lexer::Range location, Option<std::unique_ptr<Value>> value)
      : Statement(std::move(location)), value_(std::move(value)) {}

  const Option<std::unique_ptr<Value>>& value() const { return value_; }

  ~ReturnStatement() override = default;

 private:
  void accept_impl(ASTVisitor& visitor) override { visitor.visit(this); }

  Option<std::unique_ptr<Value>> value_;
};

}  // namespace ast