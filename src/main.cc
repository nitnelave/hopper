#include <iostream>
#include <sstream>
#include <string>

#include "HopperConfig.h"
#include "parser/parser.h"

int main(int argc, char* argv[]) {
  for (int i = 1; i < argc; ++i) {
    const auto* input = argv[i];  // NOLINT: "pointer arithmetics"
    auto lexer = lexer::Lexer::from_file(input);
    auto parser = parser::Parser(&lexer);
    const auto result = parser.parse();
    if (result != parser::ParseErrorCode::VALID) exit(1);
  }

  return 0;
}
