#pragma once

#include <iostream>

#include "ast/ast.h"
#include "ast/binary_operation.h"
#include "ast/block_statement.h"
#include "ast/boolean_constant.h"
#include "ast/function_call.h"
#include "ast/function_declaration.h"
#include "ast/int_constant.h"
#include "ast/return_statement.h"
#include "ast/value_statement.h"
#include "ast/variable_declaration.h"
#include "ast/variable_reference.h"

namespace ast {
class PrettyPrinterVisitor : public ASTVisitor {
 public:
  explicit PrettyPrinterVisitor(std::ostream& out) : out_(out) {}

  void visit(VariableDeclaration* node) override {
    print_indent() << (node->is_mutable() ? "mut" : "val") << ' ';
    out_ << node->id().to_string();
    if (node->type().is_ok())
      out_ << " : " << node->type().value_or_die().to_string();
    if (node->value().is_ok()) {
      out_ << " = ";
      node->value().value_or_die()->accept(*this);
    }
    out_ << ";\n";
  }

  void visit(FunctionDeclaration* node) override;

  void visit(FunctionArgumentDeclaration* node) override;

  void visit(FunctionCall* node) override {
    node->base().accept(*this);
    out_ << '(';
    auto delimiter = "";
    for (auto& arg : node->arguments()) {
      out_ << delimiter;
      arg->accept(*this);
      delimiter = ", ";
    }
    out_ << ')';
  }

  void visit(BinaryOp* node) override;

  void visit(VariableReference* node) override {
    out_ << node->id().to_string();
  }

  void visit(IntConstant* node) override { out_ << node->value(); }
  void visit(BooleanConstant* node) override {
    if (node->value())
      out_ << "true";
    else
      out_ << "false";
  }

  void visit(ReturnStatement* node) override {
    print_indent() << "return";
    if (node->value().is_ok()) {
      out_ << ' ';
      node->value().value_or_die()->accept(*this);
    }
    out_ << ";\n";
  }

  void visit(ValueStatement* node) override {
    print_indent();
    node->value()->accept(*this);
    out_ << ";\n";
  }

  void visit(BlockStatement* node) override {
    print_indent() << "{\n";
    ++indent_;
    for (auto const& statement : node->statements()) {
      statement->accept(*this);
    }
    --indent_;
    print_indent() << "}";
  }

 private:
  std::ostream& print_indent() {
    for (int i = 0; i < indent_; ++i) {
      out_ << "  ";
    }
    return out_;
  }
  std::ostream& out_;
  int indent_ = 0;
};

}  // namespace ast
