#pragma once

#include <memory>
#include <vector>

#include "ast/ast.h"
#include "ast/base_types.h"
#include "ast/statement.h"
#include "util/option.h"
#include "visitor/visitor.h"

namespace ast {

class IfStatement : public Statement {
 public:
  using ConditionList = std::vector<std::unique_ptr<Value>>;
  using BodyList = std::vector<std::unique_ptr<BlockStatement>>;
  IfStatement(lexer::Range location, std::unique_ptr<Value> condition,
              std::unique_ptr<BlockStatement> body,
              Option<std::unique_ptr<BlockStatement>> else_statement)
      : Statement(std::move(location), NodeType::IF_STATEMENT),
        condition_(std::move(condition)),
        body_(std::move(body)),
        else_statement_(std::move(else_statement)) {}

  const std::unique_ptr<Value>& condition() const { return condition_; }
  const std::unique_ptr<BlockStatement>& body() const { return body_; }
  const Option<std::unique_ptr<BlockStatement>>& else_statement() const {
    return else_statement_;
  }

  ~IfStatement() override = default;

 private:
  void accept_impl(ASTVisitor& visitor) override { visitor.visit(this); }

  std::unique_ptr<Value> condition_;
  std::unique_ptr<BlockStatement> body_;
  Option<std::unique_ptr<BlockStatement>> else_statement_;
};

}  // namespace ast
