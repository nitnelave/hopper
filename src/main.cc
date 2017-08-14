#include <libgen.h>
#include <iostream>
#include <string>

#include "HopperConfig.h"

#include "ast/module.h"
#include "codegen/codegen.h"
#include "parser/parser.h"
#include "pretty_printer/pretty_printer.h"
#include "transform/function_value_body.h"
#include "util/gflags_utils.h"
#include "util/logging.h"

// LCOV_EXCL_START: main is not tested

std::string ir_filename(const std::string& filename) {
  auto last = filename.find_last_of(".");
  if (last == std::string::npos)
    throw std::invalid_argument("Filename " + filename +
                                " does not end in '.gh'");
  return filename.substr(0, last + 1) + "ll";
}

std::string get_usage_string(const std::string& program_name) {
  return std::string(R"(gHopper compiler.
Version )") +
         ghopper_version_string + R"(

Usage: )" +
         program_name + R"( [FLAGS] SOURCES)";
}

int main(int argc, char* argv[]) {
  gflags::SetUsageMessage(get_usage_string(basename(argv[0])));  // NOLINT
  gflags::SetVersionString(ghopper_version_string);
  gflags::GFlagsWrapper w(&argc, &argv, true);

  codegen::LLVMInitializer llvm_initializer;

  int exit_code = 0;
  for (int i = 1; i < argc; ++i) {
    std::string input = argv[i];  // NOLINT: "pointer arithmetics"
    log(DEBUG) << "Processing file " << input;
    auto lexer = lexer::from_file(input);
    auto parser = parser::Parser(&lexer);
    auto result = parser.parse();
    if (!result.is_ok()) {
      std::cerr << result.to_string() << '\n';
      exit_code = 1;
      break;
    } else {
      // Transform value functions (fun a() = 3;) into statement functions
      // (fun a() { return 3; }).
      transform::FunctionValueBodyTransformer transformer;
      result.value_or_die()->accept(transformer);

      // Pretty-print the AST to standard output.
      ast::PrettyPrinterVisitor printer(std::cout);
      result.value_or_die()->accept(printer);

      // Generate the LLVM IR representation.
      codegen::CodeGenerator generator(input);
      result.value_or_die()->accept(generator);
      auto out = codegen::get_ostream_for_file(ir_filename(input));
      // Print the IR to a file.
      generator.print(*out);

      for (auto const& warning : generator.error_list().warnings()) {
        std::cerr << warning.to_string() << std::endl;
      }
    }
  }

  return exit_code;
}

// LCOV_EXCL_STOP
