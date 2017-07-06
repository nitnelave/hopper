#pragma once

#include "lexer/token.h"

namespace ast {

class ASTVisitor;

/// Base AST Node class, abstract
class ASTNode {
 public:
  explicit ASTNode(lexer::Range location) : location_(std::move(location)) {}

  /// For the visitor pattern.
  void accept(ASTVisitor& visitor) { this->accept_impl(visitor); }

  /// The range of text that this piece of code represents.
  const lexer::Range& location() const { return location_; }

  virtual ~ASTNode() = default;

 private:
  virtual void accept_impl(ASTVisitor& visitor) = 0;
  const lexer::Range location_;
};

/// Concrete node classes.
class Assignment;
class BinaryOp;
// class ForBlock;
class FunctionCall;
class FunctionDeclaration;
// class IfBlock;
// class ImportStatement;
class IntConstant;
class Return;
// class ScopeBlock;
// class UnaryOp;
class VariableDeclaration;
class VariableReference;
// class WhenBlock;
// class WhenCase;
// class WhileBlock;

/// Abstract node classes.
class Value;

class ASTVisitor {
 public:
  virtual void visit(Assignment* node) = 0;
  virtual void visit(BinaryOp* node) = 0;
  virtual void visit(FunctionCall* node) = 0;
  virtual void visit(FunctionDeclaration* node) = 0;
  virtual void visit(IntConstant* node) = 0;
  virtual void visit(Return* node) = 0;
  virtual void visit(Value* node) = 0;
  virtual void visit(VariableDeclaration* node) = 0;
  virtual void visit(VariableReference* node) = 0;
};

}  // namespace ast
