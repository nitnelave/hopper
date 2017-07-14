#include <iostream>
#include <string>

#include <gflags/gflags.h>

#include "HopperConfig.h"
#include "ast/module.h"
#include "parser/parser.h"
#include "pretty_printer/pretty_printer.h"
#include "transform/function_value_body.h"

// LCOV_EXCL_START: main is not tested

DEFINE_bool(success, false, "Whether we successfully added a flag");

int main(int argc, char* argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  if (FLAGS_success) std::cout << "Success!\n";

  int exit_code = 0;
  for (int i = 1; i < argc; ++i) {
    const auto* input = argv[i];  // NOLINT: "pointer arithmetics"
    auto lexer = lexer::from_file(input);
    auto parser = parser::Parser(&lexer);
    auto result = parser.parse();
    if (!result.is_ok()) {
      std::cerr << result.to_string() << '\n';
      exit_code = 1;
      break;
    } else {
      transform::FunctionValueBodyTransformer transformer;
      result.value_or_die()->accept(transformer);
      ast::PrettyPrinterVisitor printer(std::cout);
      result.value_or_die()->accept(printer);
    }
  }

  gflags::ShutDownCommandLineFlags();
  return exit_code;
}

// LCOV_EXCL_STOP
