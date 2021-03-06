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
#include "util/logging.h"

namespace codegen {

using namespace llvm;  // NOLINT

void CodeGenerator::visit(ast::ValueStatement* node) {
  node->value()->accept(*this);
  CHECK(gen_value_.is_ok()) << "No value generated by the visitor";
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
  llvm_function->setCallingConv(CallingConv::C);
  functions_[node] = llvm_function;

  // Name the parameters.
  CHECK(llvm_function->arg_size() == node->arguments().size())
      << "There should be as much arguments in the llvm function as in the "
         "ASTNode object";
  auto llvm_arg = llvm_function->arg_begin();
  auto gh_arg = std::begin(node->arguments());
  for (; llvm_arg != llvm_function->arg_end(); llvm_arg++, gh_arg++) {
    llvm_arg->setName((*gh_arg)->id().to_string());
    functions_args_[(*gh_arg).get()] = &*llvm_arg;
  }

  // Create body of the function.
  CHECK(node->body().is<ast::FunctionDeclaration::StatementsBody>())
      << "FunctionDeclaration body was not transformed, still a value";
  BasicBlock* body_block = BasicBlock::Create(context_, node->id().to_string(),
                                              current_function_.value_or_die());
  ir_builder_.SetInsertPoint(body_block);
  node->accept_body(*this);

  consume_return_value();
}

}  // namespace codegen
