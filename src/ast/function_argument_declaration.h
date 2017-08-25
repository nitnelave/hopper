#pragma once

#include "ast/ast.h"
#include "ast/base_types.h"
#include "ast/variable_declaration.h"
#include "visitor/visitor.h"

namespace ast {

class FunctionArgumentDeclaration : public VariableDeclaration {
 public:
  FunctionArgumentDeclaration(lexer::Range location, Identifier id,
                              Option<Type> type,
                              Option<std::unique_ptr<Value>> value, bool mut,
                              CallingConvention calling_convention)
      : VariableDeclaration(std::move(location),
                            NodeType::FUNCTION_ARGUMENT_DECLARATION,
                            std::move(id), std::move(type), std::move(value),
                            mut, calling_convention) {}

 private:
  void accept_impl(ASTVisitor& visitor) override { visitor.visit(this); }
};

}  // namespace ast
