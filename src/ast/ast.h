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

  // LCOV_EXCL_START: never called
  virtual ~ASTNode() = default;
  // LCOV_EXCL_STOP

 private:
  virtual void accept_impl(ASTVisitor& visitor) = 0;
  const lexer::Range location_;
};

/// Concrete node classes.
class Assignment;
class BinaryOp;
class BooleanConstant;
class BuiltinType;
// class ForBlock;
class FunctionArgumentDeclaration;
class FunctionCall;
class FunctionDeclaration;
// class IfBlock;
// class ImportStatement;
class IntConstant;
class Module;
class ReturnStatement;
// class ScopeBlock;
// class UnaryOp;
class BlockStatement;
class ValueStatement;
class VariableDeclaration;
class VariableReference;
// class WhenBlock;
// class WhenCase;
// class WhileBlock;

/// Abstract node classes.
class Declaration;
class Statement;
class TypeDeclaration;
class Value;

}  // namespace ast
