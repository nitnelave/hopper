#pragma once

#include <memory>

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"

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
  void visit(ast::ReturnStatement* node) override;
  void visit(ast::ValueStatement* node) override;
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
};

}  // namespace codegen
