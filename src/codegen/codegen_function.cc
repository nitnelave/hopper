#include "codegen/codegen.h"

#include "llvm/IR/CallingConv.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"

#include "ast/block_statement.h"
#include "ast/function_call.h"
#include "ast/function_declaration.h"
#include "ast/return_statement.h"
#include "ast/value.h"
#include "ast/value_statement.h"
#include "ast/variable_declaration.h"
#include "ast/variable_reference.h"

namespace codegen {

using namespace llvm;  // NOLINT

void CodeGenerator::visit(ast::ValueStatement* node) {
  node->value()->accept(*this);
  assert(gen_value_.is_ok() && "No value generated by the visitor");
}

void CodeGenerator::visit(ast::FunctionCall* node) {
  switch (node->base()->value_type()) {
    // We can access the function name.
    case ast::ValueType::VARIABLE: {
      auto variable_ref = static_cast<ast::VariableReference*>(node->base());
      auto fun_name = variable_ref->id().to_string();
      auto fun_itr = functions_.find(fun_name);
      assert(fun_itr != std::end(functions_) && "Binding should have detected this");

      // Create arguments alloca + stores.
      std::vector<Value*> args;
      auto fun_ptr = fun_itr->second.top();
      for (auto const& ast_arg : node->arguments()) {
        // TODO: handle default arguments.
        ast_arg->accept(*this);
        assert(gen_value_.is_ok() && "Expected value");
        args.push_back(gen_value_.value_or_die());
      }

      ArrayRef<Value*> llvm_args(args);
      gen_value_ = ir_builder_.CreateCall(fun_ptr, llvm_args, fun_name);
    } break;
    case ast::ValueType::RESULT:
    case ast::ValueType::CONSTANT:
      throw std::runtime_error("Unimpemented feature");
    case ast::ValueType::UNKNOWN:
      throw std::runtime_error("Shouldn't get the UNKNOWN value type");
  }
}

void CodeGenerator::visit(ast::FunctionDeclaration* node) {
  std::vector<Type*> param_types;  // fill by visiting.
  for (std::size_t i = 0; i < node->arguments().size(); ++i) {
    // TODO: put real type here.
    param_types.push_back(IntegerType::get(context_, 32));
  }

  ArrayRef<Type*> param_types_array(param_types);
  // TODO: put real return type here.
  FunctionType* t = FunctionType::get(IntegerType::get(context_, 32),
                                      param_types_array, /*isVarArg=*/false);
  Constant* c = module_->getOrInsertFunction(node->id().to_string(), t);

  Function* llvm_function = cast<Function>(c);
  current_function_ = llvm_function;
  current_function_name_ = node->id().short_name();
  llvm_function->setCallingConv(CallingConv::C);

  // Name the parameters.
  assert(llvm_function->arg_size() == node->arguments().size());
  auto llvm_arg = llvm_function->arg_begin();
  auto gh_arg = std::begin(node->arguments());
  for (; llvm_arg != llvm_function->arg_end(); llvm_arg++, gh_arg++) {
    llvm_arg->setName((*gh_arg)->id().to_string());
    // TODO: put the argument on some stack to retrieve them later ?
  }

  // Create body of the function.
  add_variable_scope_level();
  add_function_scope_level(node->id().to_string(), llvm_function);
  assert(node->body().is<ast::FunctionDeclaration::StatementsBody>() &&
         "FunctionDeclaration body was not transformed, still a value");
  node->accept_body(*this);
  remove_variable_scope_level();
  remove_function_scope_level();
}

}  // namespace codegen
