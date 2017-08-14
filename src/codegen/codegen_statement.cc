#include "codegen/codegen.h"

#include <iostream>
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include "ast/block_statement.h"
#include "ast/if_statement.h"
#include "ast/return_statement.h"
#include "ast/value.h"
#include "util/option.h"

namespace codegen {

using namespace llvm;  // NOLINT

void CodeGenerator::visit(ast::BlockStatement* node) {
  auto current_block =
      BasicBlock::Create(context_, current_function_name_, current_function_);
  ir_builder_.SetInsertPoint(current_block);

  for (auto const& statement : node->statements()) {
    statement->accept(*this);

    // TODO: plug the SSA code here for PHI nodes.

    if (has_returned_) {
      return;
    }
  }
}

void CodeGenerator::visit(ast::ReturnStatement* node) {
  if (node->value().is_ok()) {
    node->value().value_or_die()->accept(*this);
    assert(gen_value_ != nullptr && "No value generated by the visitor");
    ir_builder_.CreateRet(gen_value_);
  } else {
    ir_builder_.CreateRetVoid();
  }

  has_returned_ = true;
}

void CodeGenerator::visit(ast::IfStatement* node) {
  auto if_start_block = ir_builder_.GetInsertBlock();
  Option<BasicBlock*> ifend_block = none;

  Option<Value*> condition_value = none;
  if (node->condition().is_ok()) {
    node->condition().value_or_die()->accept(*this);
    condition_value = gen_value_;
  }

  // We generate the if block.
  node->body()->accept(*this);
  auto if_body_returned = consume_return_value();
  BasicBlock* if_block = ir_builder_.GetInsertBlock();

  bool else_body_returned = false;
  Option<BasicBlock*> else_block = none;
  Option<BasicBlock*> ifelse_block = none;

  // We generate the else block if needed.
  if (node->else_statement().is_ok()) {
    ifend_block = BasicBlock::Create(context_, "if.else", current_function_);
    ifelse_block = ifend_block;

    ir_builder_.SetInsertPoint(ifend_block.value_or_die());
    node->else_statement().value_or_die()->accept(*this);
    else_body_returned = consume_return_value();
    else_block = ir_builder_.GetInsertBlock();
  }

  bool else_only_returned =
      !condition_value.is_ok() && if_body_returned && !else_block.is_ok();
  bool if_or_elseif_returned =
      if_body_returned && else_body_returned && else_block.is_ok();
  has_returned_ = if_or_elseif_returned || else_only_returned;

  if (!has_returned_) {
    ifend_block = BasicBlock::Create(context_, "if.end", current_function_);
  }

  // We make the jump from the if start block to the bodies.
  ir_builder_.SetInsertPoint(if_start_block);
  if (condition_value.is_ok()) {
    auto equality = ir_builder_.CreateICmpNE(condition_value.value_or_die(),
                                             ir_builder_.getInt32(0));
    // Handle the if alone, or the if else.
    ir_builder_.CreateCondBr(equality, if_block,
                             ifelse_block.value_or(ifend_block.value_or_die()));
  } else {
    // Handle the else alone.
    ir_builder_.CreateBr(if_block);
  }

  // We jump from the bodies to if.end if needed.
  if (has_returned_) return;

  if (!if_body_returned) {
    ir_builder_.SetInsertPoint(if_block);
    ir_builder_.CreateBr(ifend_block.value_or_die());
  }

  if (!else_body_returned && else_block.is_ok()) {
    ir_builder_.SetInsertPoint(else_block.value_or_die());
    ir_builder_.CreateBr(ifend_block.value_or_die());
  }

  ir_builder_.SetInsertPoint(ifend_block.value_or_die());
}
}  // namespace codegen
