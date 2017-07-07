#include "ast/ast.h"
#include "ast/module.h"
#include "ast/variable_declaration.h"

namespace ast {

void ASTVisitor::visit(Assignment* /*unused*/) {}
void ASTVisitor::visit(BinaryOp* /*unused*/) {}
void ASTVisitor::visit(FunctionCall* /*unused*/) {}
void ASTVisitor::visit(FunctionDeclaration* /*unused*/) {}
void ASTVisitor::visit(IntConstant* /*unused*/) {}
void ASTVisitor::visit(Module* node) {
  for (const auto& declaration : node->top_level_declarations())
    declaration->accept(*this);
}
void ASTVisitor::visit(Return* /*unused*/) {}
void ASTVisitor::visit(Value* /*unused*/) {}
void ASTVisitor::visit(VariableDeclaration* node) {
  if (node->value().is_ok()) node->value().value_or_die()->accept(*this);
}
void ASTVisitor::visit(VariableReference* /*unused*/) {}

}  // namespace ast