#pragma once

#include <list>
#include <memory>

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

class CodeGenerator : public ast::ASTVisitor {
 public:
  using ErrorList = ast::ErrorList<ast::VisitorError>;

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
    }
  }

  // void visit(ast::VariableDeclaration* node) override;
  // void visit(ast::VariableReference* node) override;

  llvm::Module& get_module();

  void print(llvm::raw_ostream& out) const;

  const ErrorList& error_list() const { return error_list_; }

 private:
  llvm::LLVMContext context_;
  std::unique_ptr<llvm::Module> module_;
  llvm::IRBuilder<> ir_builder_;
  // Return value of visitation of a value node.
  Option<llvm::Value*> gen_value_;

  // Current function holding the blocks.
  llvm::Function* current_function_;

  // Name of the current function.
  std::string current_function_name_;

  // True if the statement has fully returned, false otherwise.
  bool has_returned_ = false;

  ErrorList error_list_;

  bool consume_return_value() {
    bool has_returned = has_returned_;
    has_returned_ = false;
    assert(has_returned_ == false &&
           "After consuming, has_returned_ should be false");
    return has_returned;
  }
};
}  // namespace codegen
