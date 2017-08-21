#pragma once

#include "visitor/visitor.h"

namespace transform {

/// Add a return statement at the end of functions returning `Void`.
class VoidFunctionReturnAdder : public ast::ASTVisitor {
 public:
  void visit(ast::FunctionDeclaration* node) override;
};
}  // namespace transform
