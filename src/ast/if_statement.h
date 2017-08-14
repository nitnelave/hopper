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
  using ConditionList = std::vector<std::unique_ptr<Value>>;
  using BodyList = std::vector<std::unique_ptr<BlockStatement>>;
  IfStatement(lexer::Range location, ConditionList conditions, BodyList bodies,
              Option<std::unique_ptr<BlockStatement>> else_statement)
      : Statement(std::move(location)),
        conditions_(std::move(conditions)),
        bodies_(std::move(bodies)),
        else_statement_(std::move(else_statement)) {
    assert(conditions.size() == bodies.size());
  }

  const ConditionList& conditions() const { return conditions_; }
  const BodyList& bodies() const { return bodies_; }
  const Option<std::unique_ptr<IfStatement>>& else_statement() const {
    return else_statement_;
  }

  ~IfStatement() override = default;

 private:
  void accept_impl(ASTVisitor& visitor) override { visitor.visit(this); }

  ConditionList conditions_;
  BodyList bodies_;
  Option<std::unique_ptr<IfStatement>> else_statement_;
};

}  // namespace ast
