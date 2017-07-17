#include "codegen/codegen.h"
#include "ast/int_constant.h"
#include "ast/return_statement.h"

namespace codegen {

using namespace llvm;  // NOLINT

void CodeGenerator::visit(ast::IntConstant* node) {
  gen_value_ =
      llvm::Constant::getIntegerValue(llvm::IntegerType::get(context_, 32),
                                      llvm::APInt(32, node->value(), true));
}

}  // namespace codegen
