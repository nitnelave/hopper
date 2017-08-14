#pragma once

#include <memory>

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"

#include "ast/module.h"
#include "visitor/visitor.h"

namespace codegen {

struct LLVMInitializer {
  LLVMInitializer();
  ~LLVMInitializer();
};

std::unique_ptr<llvm::raw_fd_ostream> get_ostream_for_file(
    const std::string& filename);

class CodeGenerator : public ast::ASTVisitor {
 public:
  explicit CodeGenerator(const std::string& name);
  // void visit(ast::Assignment* node) override;
  // void visit(ast::BinaryOp* node) override;
  // void visit(ast::FunctionArgumentDeclaration* node) override;
  // void visit(ast::FunctionCall* node) override;
  void visit(ast::FunctionDeclaration* node) override;
  void visit(ast::IntConstant* node) override;
  void visit(ast::BlockStatement* node) override;
  void visit(ast::ReturnStatement* node) override;
  void visit(ast::ValueStatement* node) override;
  void visit(ast::IfStatement* node) override;

  void visit(ast::Module* node) override {
    for (auto const& declaration : node->top_level_declarations()) {
      current_block_ = nullptr;
      declaration->accept(*this);
    }
  }

  // void visit(ast::VariableDeclaration* node) override;
  // void visit(ast::VariableReference* node) override;

  llvm::Module& get_module();

  void print(llvm::raw_ostream& out) const;

 private:
  llvm::LLVMContext context_;
  std::unique_ptr<llvm::Module> module_;
  llvm::IRBuilder<> ir_builder_;
  // Return value of visitation of a value node.
  llvm::Value* gen_value_;
  llvm::BasicBlock* current_block_;
  llvm::Function* current_function_;
  std::string current_function_name_;

  bool return_unhandled_ = false;
};

#define CONSUME_UNHANDLED_RETURN(VAR) \
  VAR = return_unhandled_;            \
  return_unhandled_ = false;
}  // namespace codegen
