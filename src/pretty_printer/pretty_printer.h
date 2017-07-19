#pragma once

#include <iostream>

#include "ast/ast.h"
#include "ast/binary_operation.h"
#include "ast/boolean_constant.h"
#include "ast/function_declaration.h"
#include "ast/int_constant.h"
#include "ast/return_statement.h"
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

  void visit(FunctionDeclaration* node) override {
    print_indent() << "fun " << node->id().to_string() << '(';
    bool print_comma = false;
    for (const auto& arg : node->arguments()) {
      if (print_comma) out_ << ',';
      arg->accept(*this);
    }
    out_ << ')';
    if (node->type().is_ok())
      out_ << " : " << node->type().value_or_die().to_string();
    out_ << ' ';
    if (node->body().is<FunctionDeclaration::StatementsBody>()) {
      out_ << "{\n";
      ++indent_;
      for (const auto& statement :
           node->body().get_unchecked<FunctionDeclaration::StatementsBody>()) {
        statement->accept(*this);
        out_ << '\n';
      }
      --indent_;
      print_indent() << "}";
    } else {
      out_ << "= ";
      node->body().get_unchecked<FunctionDeclaration::ValueBody>()->accept(
          *this);
      out_ << ';';
    }
    out_ << '\n';
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
    out_ << ';';
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
