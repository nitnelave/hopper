#pragma once

#include <vector>

#include "ast/ast.h"
#include "ast/base_types.h"
#include "error/error.h"
#include "lexer/token.h"
#include "visitor/error_visitor.h"

namespace typechecker {
class TypeChecker : public ast::VisitorWithErrors<> {
 public:
  using ErrorList = ast::ErrorList<ast::VisitorError>;

  void visit(ast::BooleanConstant* node) override;
  void visit(ast::IntConstant* node) override;
  void visit(ast::BinaryOp* node) override;
  void visit(ast::FunctionDeclaration* node) override;
  void visit(ast::ReturnStatement* node) override;
  void visit(ast::VariableReference* node) override;
  void visit(ast::LocalVariableDeclaration* node) override;

 private:
  // We may have to turn that into a stack to support nested functions.
  Option<ast::Type> function_return_type_ = none;
};
}  // namespace typechecker
