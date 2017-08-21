#pragma once

#include <stack>
#include <vector>

#include "util/logging.h"
#include "visitor/error_visitor.h"

namespace transform {

/// Add a return statement at the end of functions returning `Void`.
class VariableDestructorAdder : public ast::VisitorWithErrors<> {
 public:
  using ScopeVariables = std::vector<ast::VariableDeclaration*>;
  using ScopeStack = std::vector<ScopeVariables>;
  void visit(ast::FunctionDeclaration* node) override;
  void visit(ast::BlockStatement* node) override;

 private:
  void enter_scope() { variables_in_scope_.emplace_back(); }

  ScopeVariables& current_scope() {
    CHECK(!variables_in_scope_.empty());
    return variables_in_scope_.back();
  }

  void exit_scope() {
    CHECK(!variables_in_scope_.empty());
    variables_in_scope_.pop_back();
  }

  // Process a block, with potentially function arguments if it's the
  // function's main body.
  void visit_block(
      ast::BlockStatement* node,
      const std::vector<ast::VariableDeclaration*>& function_arguments = {});

  ScopeStack variables_in_scope_;
};
}  // namespace transform
