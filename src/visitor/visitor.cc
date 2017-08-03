#include "visitor/visitor.h"
#include "ast/binary_operation.h"
#include "ast/block_statement.h"
#include "ast/function_declaration.h"
#include "ast/module.h"
#include "ast/return_statement.h"
#include "ast/variable_declaration.h"

namespace ast {

void ASTVisitor::visit(Assignment* /*unused*/) {}
void ASTVisitor::visit(BooleanConstant* /*unused*/) {}
void ASTVisitor::visit(BinaryOp* node) {
  node->left_value().accept(*this);
  node->right_value().accept(*this);
}
void ASTVisitor::visit(BuiltinType* /*unused*/) {}
void ASTVisitor::visit(FunctionArgumentDeclaration* /*unused*/) {}
void ASTVisitor::visit(FunctionCall* /*unused*/) {}
void ASTVisitor::visit(FunctionDeclaration* node) {
  for (const auto& argument : node->arguments()) {
    argument->accept(*this);
  }
  node->accept_body(*this);
}
void ASTVisitor::visit(IntConstant* /*unused*/) {}
void ASTVisitor::visit(Module* node) {
  for (const auto& declaration : node->top_level_declarations())
    declaration->accept(*this);
}
void ASTVisitor::visit(ReturnStatement* node) {
  if (node->value().is_ok()) node->value().value_or_die()->accept(*this);
}
void ASTVisitor::visit(VariableDeclaration* node) {
  if (node->value().is_ok()) node->value().value_or_die()->accept(*this);
}
void ASTVisitor::visit(VariableReference* /*unused*/) {}

void ASTVisitor::visit(BlockStatement* node) {
  for (const auto& statement : node->statements()) {
    statement->accept(*this);
  }
}
}  // namespace ast
