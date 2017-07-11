#pragma once

#include <iostream>

#include "ast/ast.h"
#include "ast/int_constant.h"
#include "ast/variable_declaration.h"
#include "ast/variable_reference.h"

namespace ast {
class PrettyPrinterVisitor : public ASTVisitor {
 public:
  explicit PrettyPrinterVisitor(std::ostream& out) : out_(out) {}

  void visit(VariableDeclaration* node) override {
    out_ << (node->is_mutable() ? "mut" : "val") << ' ';
    out_ << node->id().to_string();
    if (node->type().is_ok())
      out_ << " : " << node->type().value_or_die().to_string();
    if (node->value().is_ok()) {
      out_ << " = ";
      node->value().value_or_die()->accept(*this);
    }
    out_ << ";\n";
  }

  void visit(VariableReference* node) override {
    out_ << node->id().to_string();
  }

  void visit(IntConstant* node) override { out_ << node->value(); }

 private:
  std::ostream& out_;
};

}  // namespace ast
