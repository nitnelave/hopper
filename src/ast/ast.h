#pragma once

#include "lexer/token.h"

namespace ast {

// Tag to get the type of a node.
enum class NodeType {
  ASSIGNMENT,
  BINARY_OP,
  BOOLEAN_CONSTANT,
  BUILTIN_TYPE,
  FOR_BLOCK,
  FUNCTION_ARGUMENT_DECLARATION,
  FUNCTION_CALL,
  FUNCTION_DECLARATION,
  IF_STATEMENT,
  IMPORT_STATEMENT,
  INT_CONSTANT,
  MODULE,
  RETURN_STATEMENT,
  SCOPE_BLOCK,
  UNARY_OP,
  BLOCK_STATEMENT,
  LOCAL_VARIABLE_DECLARATION,
  VALUE_STATEMENT,
  VARIABLE_DESTRUCTION,
  VARIABLE_REFERENCE,
  WHEN_BLOCK,
  WHEN_CASE,
  WHILE_BLOCK,
};

class ASTVisitor;

/// Base AST Node class, abstract
class ASTNode {
 public:
  explicit ASTNode(lexer::Range location, NodeType node_type)
      : location_(std::move(location)), node_type_(node_type) {}

  /// For the visitor pattern.
  void accept(ASTVisitor& visitor) { this->accept_impl(visitor); }

  /// The range of text that this piece of code represents.
  const lexer::Range& location() const { return location_; }

  NodeType node_type() const { return node_type_; }

  // LCOV_EXCL_START: never called
  virtual ~ASTNode() = default;
  // LCOV_EXCL_STOP

 private:
  virtual void accept_impl(ASTVisitor& visitor) = 0;
  const lexer::Range location_;
  NodeType node_type_;
};

/// Concrete node classes.
class Assignment;
class BlockStatement;
class BinaryOp;
class BooleanConstant;
class BuiltinType;
// class ForBlock;
class FunctionArgumentDeclaration;
class FunctionCall;
class FunctionDeclaration;
class IfStatement;
// class ImportStatement;
class IntConstant;
class LocalVariableDeclaration;
class Module;
class ReturnStatement;
// class ScopeBlock;
// class UnaryOp;
class ValueStatement;
class VariableDestruction;
class VariableReference;
// class WhenBlock;
// class WhenCase;
// class WhileBlock;

/// Abstract node classes.
class Declaration;
class Statement;
class TypeDeclaration;
class VariableDeclaration;
class Value;

}  // namespace ast
