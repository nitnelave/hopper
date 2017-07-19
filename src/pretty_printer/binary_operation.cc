#include "pretty_printer/pretty_printer.h"
namespace ast {

void PrettyPrinterVisitor::visit(BinaryOp* node) {
  out_ << '(';
  node->left_value().accept(*this);
  out_ << ' ';
  out_ << node->operation();
  out_ << ' ';
  node->right_value().accept(*this);
  out_ << ')';
}
}  // namespace ast
