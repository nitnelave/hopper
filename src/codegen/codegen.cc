#include "codegen/codegen.h"

#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

#include "ast/function_declaration.h"

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

namespace {
template <typename ScopeType, typename Symbols>
void remove_scope_level(ScopeType& stacked_scopes, Symbols& symbols,
                        CodeGenerator::SymbolLocations& locations) {
  assert(!stacked_scopes.empty() && "There should be at least one scope level");
  auto current_scope = stacked_scopes.top();
  for (auto const& symbol : current_scope) {
    auto sym_itr = symbols.find(symbol);
    assert(sym_itr != std::end(symbols) && "The symbol should be present");

    // Remove from symbol locations.
    auto sym_ptr = sym_itr->second.top();
    locations.erase(locations.find(sym_ptr));

    // Remove one scope.
    sym_itr->second.pop();

    // No more scope available.
    if (sym_itr->second.empty()) {
      symbols.erase(sym_itr);
    }
  }

  stacked_scopes.pop();
}

template <typename ScopeType, typename Symbols, typename SymbolType>
SymbolType add_var_to_scope(ScopeType& stacked_scopes, Symbols& symbols,
                            const std::string& sym_name,
                            std::function<SymbolType()> sym_factory,
                            CodeGenerator::SymbolLocations& locations,
                            CodeGenerator::ErrorList& error_list) {
  auto current_scope = stacked_scopes.top();
  assert(current_scope.find(sym_name) == std::end(current_scope) &&
         "Binding visitor should have detected that behavior");

  auto sym_itr = symbols.find(sym_name);
  if (sym_itr != std::end(symbols)) {
    auto sym_ptr = sym_itr->second.top();
    error_list.add_warning(locations[sym_ptr],
                           "Variable declaration shadowing another one");
  } else {
    symbols[sym_name] = {};
  }

  current_scope.insert(sym_name);
  // TODO: real types
  auto symbol = sym_factory();
  symbols[sym_name].push(symbol);
  return symbol;
}
}  // namespace

void CodeGenerator::add_variable_scope_level() { scoped_variables_.push({}); }

void CodeGenerator::remove_variable_scope_level() {
  remove_scope_level<ScopedVariables, Variables>(scoped_variables_, variables_,
                                                 sym_locations_);
}

AllocaInst* CodeGenerator::add_variable_to_scope(const std::string& var_name) {
  return add_var_to_scope<ScopedVariables, Variables, AllocaInst*>(
      scoped_variables_, variables_, var_name,
      [this, &var_name]() {
        return ir_builder_.CreateAlloca(IntegerType::get(context_, 32), 0,
                                        var_name);
      },
      sym_locations_, error_list_);
}

void CodeGenerator::add_function_scope_level(const std::string& fun_name,
                                             Function* llvm_function,
                                             ast::FunctionDeclaration* node) {
  scoped_fun_args_.push({});

  add_function_to_scope(fun_name, llvm_function);
  add_function_args_to_scope(llvm_function, node);
}

void CodeGenerator::remove_function_scope_level() {
  remove_scope_level<ScopedFunctionsArgs, FunctionsArgs>(
      scoped_fun_args_, function_args_, sym_locations_);
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

void CodeGenerator::add_function_args_to_scope(Function* function,
                                               ast::FunctionDeclaration* node) {
  auto ast_arg = std::begin(node->arguments());
  for (auto& arg : function->args()) {
    Value* arg_value = &arg;
    auto arg_name = arg_value->getName().str();

    add_var_to_scope<ScopedFunctionsArgs, FunctionsArgs, Value*>(
        scoped_fun_args_, function_args_, arg_name,
        [arg_value]() { return arg_value; }, sym_locations_, error_list_);
    sym_locations_[arg_value] = (*ast_arg)->location();
    ast_arg++;
  }
}

}  // namespace codegen
