#pragma once

#include "ast/ast.h"

namespace transform {

/// Replace the body functions with only a value in their body by a simple
/// return statement of that value.
class FunctionValueBodyTransformer : public ast::ASTVisitor {
 public:
  void visit(ast::FunctionDeclaration* node) override;
};
}  // namespace transform
