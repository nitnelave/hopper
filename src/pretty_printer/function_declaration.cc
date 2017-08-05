#include "pretty_printer/pretty_printer.h"
namespace ast {

void PrettyPrinterVisitor::visit(FunctionArgumentDeclaration* node) {
  if (node->is_mutable())
    out_ << "mut";
  else
    out_ << "val";
  out_ << ' ' << node->id().short_name();
  out_ << ": ";
  out_ << node->type().id().short_name();
}

void PrettyPrinterVisitor::visit(FunctionDeclaration* node) {
  print_indent() << "fun " << node->id().to_string() << '(';
  bool print_comma = false;
  for (const auto& arg : node->arguments()) {
    if (print_comma) out_ << ", ";
    arg->accept(*this);
    print_comma = true;
  }
  out_ << ')';
  if (node->type().is_ok())
    out_ << " : " << node->type().value_or_die().to_string();
  out_ << ' ';

  if (node->body().is<FunctionDeclaration::StatementsBody>()) {
    node->body().get_unchecked<FunctionDeclaration::StatementsBody>()->accept(
        *this);
    --indent_;
  } else {
    out_ << "= ";
    node->body().get_unchecked<FunctionDeclaration::ValueBody>()->accept(*this);
    out_ << ';';
  }
  out_ << '\n';
}
}  // namespace ast
