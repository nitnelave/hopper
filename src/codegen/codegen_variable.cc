#include "codegen/codegen.h"

#include <iostream>

#include "ast/local_variable_declaration.h"
#include "ast/value.h"
#include "ast/variable_reference.h"

namespace codegen {
using namespace llvm;  // NOLINT

void CodeGenerator::visit(ast::LocalVariableDeclaration* node) {
  auto var_name = node->id().to_string();
  auto alloca = add_variable_to_scope(var_name);

  if (node->value().is_ok()) {
    node->value().value_or_die()->accept(*this);
    assert(gen_value_.is_ok() &&
           "The variable assignment should have generate a value");
    ir_builder_.CreateStore(gen_value_.value_or_die(), alloca);
  }
}

void CodeGenerator::visit(ast::VariableReference* node) {
  auto var_name = node->id().to_string();

  auto var_itr = variables_.find(var_name);
  auto arg_itr = function_args_.find(var_name);

  assert((var_itr != std::end(variables_) ||
          arg_itr != std::end(function_args_)) &&
         "Binding visitor should have detected that behavior");

  if (var_itr != std::end(variables_)) {
    auto alloca = var_itr->second.top();
    gen_value_ = ir_builder_.CreateLoad(alloca);
  } else {
    gen_value_ = arg_itr->second.top();
  }
}

}  // namespace codegen
