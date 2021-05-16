#include "pretty_printer/pretty_printer.h"

#include "ast/extern_function_declaration.h"

namespace ast {

void PrettyPrinterVisitor::visit(FunctionArgumentDeclaration* node) {
  if (node->is_mutable())
    out_ << "mut";
  else
    out_ << "val";
  out_ << ' ' << node->id().short_name();

  if (node->type().is_ok()) {
    out_ << ": ";
    out_ << node->type().value_or_die().id().short_name();
  }
}

void PrettyPrinterVisitor::visit(ExternFunctionDeclaration* node) {
  out_ << "extern ";

  if (node->calling_convention() == ast::CallingConvention::C) {
    out_ << "\"C\" ";
  }

  visit(static_cast<FunctionDeclaration*>(node));
  out_ << ';';
}

void PrettyPrinterVisitor::visit(FunctionDeclaration* node) {
  out_ << "fun " << node->id().to_string() << '(';
  bool print_comma = false;
  for (const auto& arg : node->arguments()) {
    if (print_comma) out_ << ", ";
    arg->accept(*this);
    print_comma = true;
  }
  out_ << ')';
  if (node->type().is_ok())
    out_ << " : " << node->type().value_or_die().to_string();

  if (node->body().is<NoneType>()) {
    return;
  }

  out_ << ' ';

  const auto& body = node->body();
  if (body.is<FunctionDeclaration::StatementsBody>()) {
    out_ << "{\n";
    ++indent_;
    for (auto const& statement :
         body.get_unchecked<FunctionDeclaration::StatementsBody>()
             ->statements()) {
      print_indent();
      statement->accept(*this);
      out_ << '\n';
    }
    --indent_;
    print_indent() << "}";
  } else {
    out_ << "= ";
    body.get_unchecked<FunctionDeclaration::ValueBody>()->accept(*this);
    out_ << ';';
  }
}
}  // namespace ast
