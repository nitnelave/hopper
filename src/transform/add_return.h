#pragma once

#include "visitor/error_visitor.h"
#include "visitor/visitor.h"

namespace transform {

/// Add a return statement at the end of functions returning `Void`.
class VoidFunctionReturnAdder : public ast::ASTVisitor {
 public:
  void visit(ast::FunctionDeclaration* node) override;
  void visit(ast::IfStatement* node) override;
  void visit(ast::ReturnStatement* node) override;
  void visit(ast::BlockStatement* node) override;

  const ast::ErrorList<>& error_list() const {
    return error_list_;
  }

 private:
  bool has_returned_ = false;
  ast::ErrorList<> error_list_;
};
}  // namespace transform
