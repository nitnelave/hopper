#pragma once

#include <memory>
#include <vector>

#include "ast/base_types.h"
#include "ast/block_statement.h"
#include "ast/function_argument_declaration.h"
#include "ast/function_declaration.h"
#include "ast/value.h"
#include "util/option.h"
#include "visitor/visitor.h"

namespace ast {

enum CallingConvention { NORMAL = 0, C = 1 };

class ExternFunctionDeclaration : public FunctionDeclaration {
 public:
  /// Function declaration only.
  ExternFunctionDeclaration(lexer::Range location, Identifier id,
                            ArgumentList arguments, Option<Type> type,
                            CallingConvention calling_convention)
      : FunctionDeclaration(std::move(location), std::move(id),
                            std::move(arguments), std::move(type)),
        calling_convention_(calling_convention) {}

  const CallingConvention& calling_convention() const {
    return calling_convention_;
  }

  ~ExternFunctionDeclaration() override = default;

 private:
  void accept_impl(ASTVisitor& visitor) override { visitor.visit(this); }
  CallingConvention calling_convention_ = CallingConvention::NORMAL;
};

}  // namespace ast
