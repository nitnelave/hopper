#include "llvm/IR/CallingConv.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"

#include "ast/block_statement.h"
#include "ast/function_declaration.h"
#include "ast/return_statement.h"
#include "ast/value.h"
#include "ast/value_statement.h"
#include "ast/variable_declaration.h"
#include "codegen/codegen.h"

namespace codegen {

using namespace llvm;  // NOLINT

void CodeGenerator::visit(ast::ValueStatement* node) {
  node->value()->accept(*this);
  assert(gen_value_ != nullptr && "No value generated by the visitor");
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
  Constant* c = module_->getOrInsertFunction(node->id().short_name(), t);

  Function* llvm_function = cast<Function>(c);
  current_function_ = llvm_function;
  current_function_name_ = node->id().short_name();
  llvm_function->setCallingConv(CallingConv::C);

  // Name the parameters.
  assert(llvm_function->arg_size() == node->arguments().size());
  auto llvm_arg = llvm_function->arg_begin();
  auto gh_arg = std::begin(node->arguments());
  for (; llvm_arg != llvm_function->arg_end(); llvm_arg++, gh_arg++) {
    llvm_arg->setName((*gh_arg)->id().short_name());
    // TODO: put the argument on some stack to retrieve them later ?
  }

  assert(node->body().is<ast::FunctionDeclaration::StatementsBody>() &&
         "FunctionDeclaration body was not transformed, still a value");
  node->accept_body(*this);
}

}  // namespace codegen
