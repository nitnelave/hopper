#include "codegen/codegen.h"

#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
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
      ir_builder_(context_, ConstantFolder()),
      gen_value_(none) {
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

void CodeGenerator::add_variable_scope_level() { scoped_variables_.push({}); }

void CodeGenerator::remove_variable_scope_level() {
  assert(!scoped_variables_.empty() &&
         "There should be at least one scope level");
  for (auto const& var : scoped_variables_.top()) {
    auto var_itr = variables_.find(var);
    assert(var_itr != std::end(variables_) && "The variable should be present");

    // Remove one scope.
    var_itr->second.pop();

    // No more scope available.
    if (var_itr->second.empty()) {
      variables_.erase(var_itr);
    }
  }

  scoped_variables_.pop();
}

AllocaInst* CodeGenerator::add_variable_to_scope(const std::string& var_name) {
  auto current_scope = scoped_variables_.top();

  assert(current_scope.find(var_name) == std::end(current_scope) &&
         "Binding visitor should have detected that behavior");

  auto var_itr = variables_.find(var_name);
  if (var_itr != std::end(variables_)) {
    // TODO: add the warning.
    // error_list_.add_warning(node->location(), "Variable declaration shadowing
    // another one");
  } else {
    variables_[var_name] = {};
  }

  current_scope.insert(var_name);
  // TODO: real types
  auto alloca =
      ir_builder_.CreateAlloca(IntegerType::get(context_, 32), 0, var_name);
  variables_[var_name].push(alloca);
  return alloca;
}

void CodeGenerator::add_function_scope_level(const std::string& fun_name, Function* llvm_function) {
  scoped_fun_args_.push({});

  add_function_to_scope(fun_name, llvm_function);
  add_function_args_to_scope(llvm_function);
}

void CodeGenerator::remove_function_scope_level() {
  assert(!scoped_fun_args_.empty() &&
         "There should be at least one scope level");
  for (auto const& var : scoped_fun_args_.top()) {
    auto var_itr = function_args_.find(var);
    assert(var_itr != std::end(function_args_) && "The variable should be present");

    // Remove one scope.
    var_itr->second.pop();

    // No more scope available.
    if (var_itr->second.empty()) {
      function_args_.erase(var_itr);
    }
  }

  scoped_fun_args_.pop();
}

void CodeGenerator::add_function_to_scope(const std::string& fun_name,
                                          Function* function) {
  if (functions_.find(fun_name) != std::end(functions_)) {
    // TODO: issue warning that we are shadowing one.
  } else {
    functions_[fun_name] = {};
  }

  functions_[fun_name].push(function);
}

void CodeGenerator::add_function_args_to_scope(Function* function) {
  for (auto& arg : function->args()) {
    Value* arg_value = &arg;
    auto arg_name = arg_value->getName().str();

    scoped_fun_args_.top().insert(arg_name);

    if (function_args_.find(arg_name) != std::end(function_args_)) {
      // TODO: issue warning for shadowing.
    } else {
      function_args_[arg_name] = {};
    }

    function_args_[arg_name].push(arg_value);
  }
}

}  // namespace codegen
