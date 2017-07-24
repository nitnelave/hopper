#pragma once

#include <vector>

#include "ast/ast.h"
#include "error/error.h"
#include "lexer/token.h"
#include "visitor/error_visitor.h"

namespace typechecker {
class TypeChecker : public ast::ASTVisitor {
 public:
  using ErrorList = ast::ErrorList<ast::VisitorError>;

  const ErrorList& error_list() const { return error_list_; }

  void visit(ast::BooleanConstant* node) override;
  void visit(ast::IntConstant* node) override;
  void visit(ast::BinaryOp* node) override;
  void visit(ast::FunctionDeclaration* node) override;

 private:
  ErrorList error_list_;
};
}  // namespace typechecker
