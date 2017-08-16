#include "codegen/codegen.h"

#include "ast/local_variable_declaration.h"
#include "ast/value.h"
#include "ast/variable_reference.h"
#include "util/logging.h"

namespace codegen {
using namespace llvm;  // NOLINT

void CodeGenerator::visit(ast::LocalVariableDeclaration* node) {
  auto var_name = node->id().to_string();

  if (current_function_.is_ok()) {
    // Scoped variable to put on the stack.
    auto alloca = ir_builder_.CreateAlloca(IntegerType::get(context_, 32),
                                           nullptr, var_name);
    variables_[node] = alloca;

    if (node->value().is_ok()) {
      node->value().value_or_die()->accept(*this);
      CHECK(gen_value_.is_ok())
          << "The variable assignment should have generate a value";
      ir_builder_.CreateStore(gen_value_.value_or_die(), alloca);
    }
  } else {
    // We have to declare a global variable.
    module_->getOrInsertGlobal(var_name, IntegerType::get(context_, 32));
    auto global_var = module_->getNamedGlobal(var_name);
    global_var->setLinkage(GlobalValue::CommonLinkage);
  }
}

void CodeGenerator::visit(ast::VariableReference* node) {
  auto var_name = node->id().to_string();

  CHECK(node->resolution().is_ok()) << "Resolution should be done";

  auto declaration = node->resolution().value_or_die();

  auto var_itr = variables_.find(declaration);
  auto arg_itr = functions_args_.find(declaration);
  auto global_var = module_->getNamedGlobal(var_name);

  CHECK(var_itr != std::end(variables_) ||
        arg_itr != std::end(functions_args_) || global_var != nullptr)
      << "Variable reference should be in scopes";

  if (var_itr != std::end(variables_)) {
    gen_value_ = ir_builder_.CreateLoad(var_itr->second);
  } else if (global_var != nullptr) {
    gen_value_ = global_var;
  } else {
    gen_value_ = arg_itr->second;
  }
}

}  // namespace codegen
