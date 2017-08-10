#pragma once

#include <memory>

#include "ast/ast.h"
#include "ast/base_types.h"
#include "ast/statement.h"
#include "util/option.h"
#include "visitor/visitor.h"

namespace ast {

class IfStatement : public Statement {
 public:
  IfStatement(lexer::Range location, Option<std::unique_ptr<Value>> condition,
              std::unique_ptr<BlockStatement> body,
              Option<std::unique_ptr<IfStatement>> elseStatement)
      : Statement(std::move(location)),
        condition_(std::move(condition)),
        body_(std::move(body)),
        else_statement_(std::move(elseStatement)) {}

  const Option<std::unique_ptr<Value>>& condition() const { return condition_; }
  const std::unique_ptr<BlockStatement>& body() const { return body_; }
  const Option<std::unique_ptr<IfStatement>>& else_statement() const {
    return else_statement_;
  }

  ~IfStatement() override = default;

 private:
  void accept_impl(ASTVisitor& visitor) override { visitor.visit(this); }

  Option<std::unique_ptr<Value>> condition_;
  std::unique_ptr<BlockStatement> body_;
  Option<std::unique_ptr<IfStatement>> else_statement_;
};

}  // namespace ast
