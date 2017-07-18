#pragma once

#include <string>

#include "lexer/lexer.h"

struct ExpectedError {
  std::string message;
  lexer::Range range;
};

bool starts_with(const std::string& haystack, const std::string& needle);

bool ends_with(const std::string& haystack, const std::string& needle);

std::string read_file(const std::string& filename);
