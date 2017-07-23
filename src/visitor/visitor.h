#pragma once

#include "ast/ast.h"

namespace ast {

class ASTVisitor {
 public:
  virtual void visit(Assignment* node);
  virtual void visit(BooleanConstant* node);
  virtual void visit(BinaryOp* node);
  virtual void visit(BuiltinType* node);
  virtual void visit(FunctionArgumentDeclaration* node);
  virtual void visit(FunctionCall* node);
  virtual void visit(FunctionDeclaration* node);
  virtual void visit(IntConstant* node);
  virtual void visit(Module* node);
  virtual void visit(ReturnStatement* node);
  virtual void visit(VariableDeclaration* node);
  virtual void visit(VariableReference* node);
};

}  // namespace ast
