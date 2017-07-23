#include "codegen/codegen.h"

#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Target/TargetOptions.h"

using namespace llvm;  // NOLINT

namespace codegen {

LLVMInitializer::LLVMInitializer() {
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmParser();
  llvm::InitializeNativeTargetAsmPrinter();
}

LLVMInitializer::~LLVMInitializer() {
#if LLVM_VERSION_MAJOR > 3 || \
    (LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR > 8)
  llvm::llvm_shutdown();
#endif
}

std::unique_ptr<raw_fd_ostream> get_ostream_for_file(
    const std::string& filename) {
  std::error_code error_code;
  return std::make_unique<raw_fd_ostream>(filename, error_code,
                                          llvm::sys::fs::F_None);
}

CodeGenerator::CodeGenerator(const std::string& name)
    : module_(std::make_unique<Module>(name, context_)),
      ir_builder_(context_, ConstantFolder()) {
  auto target_triple = sys::getDefaultTargetTriple();
  module_->setTargetTriple(target_triple);
  std::string error;

  auto target = TargetRegistry::lookupTarget(target_triple, error);

  // Print an error and exit if we couldn't find the requested target.
  // This generally occurs if we've forgotten to initialise the
  // TargetRegistry or we have a bogus target triple.
  if (target == nullptr)
    throw std::runtime_error("Could not find Target for code generation");

  auto cpu = "generic";
  auto features = "";

  TargetOptions opt;
#if LLVM_VERSION_MAJOR > 3 || \
    (LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR > 8)
  auto rm = Optional<Reloc::Model>();
#else
  auto rm = Reloc::Model();
#endif
  std::unique_ptr<TargetMachine> target_machine(
      target->createTargetMachine(target_triple, cpu, features, opt, rm));
  module_->setDataLayout(target_machine->createDataLayout());
}

Module& CodeGenerator::get_module() {
  llvm::verifyModule(*module_);
  return *module_;
}
void CodeGenerator::print(raw_ostream& out) const {
  llvm::verifyModule(*module_);
  out << *module_;
}
}  // namespace codegen
