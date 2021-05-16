#include "visitor/visitor.h"

#include "ast/binary_operation.h"
#include "ast/block_statement.h"
#include "ast/extern_function_declaration.h"
#include "ast/extern_variable_declaration.h"
#include "ast/function_declaration.h"
#include "ast/if_statement.h"
#include "ast/local_variable_declaration.h"
#include "ast/module.h"
#include "ast/return_statement.h"
#include "ast/value_statement.h"

namespace ast {

// We can use directly `visit` instead of `accept` when we know the concrete
// type. If the type is changed to an abstract one, it will fail to compile, so
// we are safe.

void ASTVisitor::visit(Assignment* /*unused*/) {}
void ASTVisitor::visit(BooleanConstant* /*unused*/) {}
void ASTVisitor::visit(BinaryOp* node) {
  node->left_value().accept(*this);
  node->right_value().accept(*this);
}
void ASTVisitor::visit(BuiltinType* /*unused*/) {}
void ASTVisitor::visit(FunctionArgumentDeclaration* /*unused*/) {}
void ASTVisitor::visit(FunctionCall* /*unused*/) {}

void ASTVisitor::visit(ExternFunctionDeclaration* node) {
  visit(static_cast<FunctionDeclaration*>(node));
}

void ASTVisitor::visit(FunctionDeclaration* node) {
  for (const auto& argument : node->arguments()) {
    visit(argument.get());
  }
  node->accept_body(*this);
}

void ASTVisitor::visit(IfStatement* node) {
  node->condition()->accept(*this);
  visit(node->body().get());
  if (node->else_statement().is_ok())
    visit(node->else_statement().value_or_die().get());
}

void ASTVisitor::visit(IntConstant* /*unused*/) {}
void ASTVisitor::visit(Module* node) {
  for (const auto& declaration : node->top_level_declarations())
    declaration->accept(*this);
}
void ASTVisitor::visit(ReturnStatement* node) {
  if (node->value().is_ok()) node->value().value_or_die()->accept(*this);
}
void ASTVisitor::visit(LocalVariableDeclaration* node) {
  if (node->value().is_ok()) node->value().value_or_die()->accept(*this);
}

void ASTVisitor::visit(ExternVariableDeclaration* node) {
  node->LocalVariableDeclaration::accept(*this);
}

void ASTVisitor::visit(VariableReference* /*unused*/) {}

void ASTVisitor::visit(BlockStatement* node) {
  for (const auto& statement : node->statements()) {
    statement->accept(*this);
  }
}

void ASTVisitor::visit(ValueStatement* node) { node->value()->accept(*this); }

}  // namespace ast
