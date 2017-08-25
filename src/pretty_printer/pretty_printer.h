#pragma once

#include <iostream>

#include "ast/ast.h"
#include "ast/binary_operation.h"
#include "ast/block_statement.h"
#include "ast/boolean_constant.h"
#include "ast/extern_variable_declaration.h"
#include "ast/function_call.h"
#include "ast/function_declaration.h"
#include "ast/if_statement.h"
#include "ast/int_constant.h"
#include "ast/local_variable_declaration.h"
#include "ast/module.h"
#include "ast/return_statement.h"
#include "ast/value_statement.h"
#include "ast/variable_reference.h"

namespace ast {
class PrettyPrinterVisitor : public ASTVisitor {
 public:
  explicit PrettyPrinterVisitor(std::ostream& out) : out_(out) {}

  void visit(Module* node) override {
    for (auto const& decl : node->top_level_declarations()) {
      decl->accept(*this);
      out_ << "\n";
    }
  }

  void visit(ExternVariableDeclaration* node) override {
    out_ << "extern ";
    visit(static_cast<LocalVariableDeclaration*>(node));
  }

  void visit(LocalVariableDeclaration* node) override {
    out_ << (node->is_mutable() ? "mut" : "val") << ' ';
    out_ << node->id().to_string();
    if (node->type().is_ok())
      out_ << " : " << node->type().value_or_die().to_string();
    if (node->value().is_ok()) {
      out_ << " = ";
      node->value().value_or_die()->accept(*this);
    }
    out_ << ";";
  }

  void visit(ExternFunctionDeclaration* node) override;

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
    out_ << "return";
    if (node->value().is_ok()) {
      out_ << ' ';
      node->value().value_or_die()->accept(*this);
    }
    out_ << ";";
  }

  void visit(ValueStatement* node) override {
    node->value()->accept(*this);
    out_ << ";";
  }

  void visit(IfStatement* node) override {
    out_ << "if (";
    node->condition()->accept(*this);
    out_ << ") ";

    node->body()->accept(*this);

    if (node->else_statement().is_ok()) {
      out_ << " else ";
      node->else_statement().value_or_die()->accept(*this);
    }
  }

  void visit(BlockStatement* node) override {
    out_ << "{\n";
    ++indent_;
    for (auto const& statement : node->statements()) {
      print_indent();
      statement->accept(*this);
      out_ << '\n';
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
