#pragma once

#include <memory>
#include <unordered_map>

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"

#include "ast/module.h"
#include "error/error.h"
#include "visitor/error_visitor.h"
#include "visitor/visitor.h"

namespace codegen {

struct LLVMInitializer {
  LLVMInitializer();
  ~LLVMInitializer();
};

std::unique_ptr<llvm::raw_fd_ostream> get_ostream_for_file(
    const std::string& filename);

class CodeGenerator : public ast::VisitorWithErrors<> {
  using Variables = std::unordered_map<ast::Declaration*, llvm::AllocaInst*>;
  using Functions = std::unordered_map<ast::Declaration*, llvm::Function*>;
  using FunctionsArgs = std::unordered_map<ast::Declaration*, llvm::Value*>;

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
      declaration->accept(*this);
      current_function_ = none;
      gen_value_ = none;
    }
  }

  void visit(ast::LocalVariableDeclaration* node) override;
  void visit(ast::VariableReference* node) override;

  llvm::Module& get_module();

  void print(llvm::raw_ostream& out) const;

 private:
  llvm::LLVMContext context_;
  std::unique_ptr<llvm::Module> module_;
  llvm::IRBuilder<> ir_builder_;
  // Return value of visitation of a value node.
  Option<llvm::Value*> gen_value_;

  /// Keeps the associations of ast::LocalVariableDeclaration to
  /// llvm::AllocaInst
  Variables variables_;

  /// Keeps the associations of ast::FunctionDeclaration to llvm::Function
  Functions functions_;

  /// Keeps the associations of ast::FunctionArgumentDeclaration to llvm::Value
  FunctionsArgs functions_args_;

  // Current function holding the blocks.
  Option<llvm::Function*> current_function_;

  // True if the statement has fully returned, false otherwise.
  bool has_returned_ = false;

  bool consume_return_value() {
    bool has_returned = has_returned_;
    has_returned_ = false;
    assert(has_returned_ == false &&
           "After consuming, has_returned_ should be false");
    return has_returned;
  }
};
}  // namespace codegen
