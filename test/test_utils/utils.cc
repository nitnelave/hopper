#include "test_utils/utils.h"

std::string crop_first_lines(const std::string& text, int number_of_lines) {
  for (size_t i = 0; i < text.size(); ++i) {
    if (text[i] == '\n') {
      --number_of_lines;
      if (number_of_lines == 0) {
        return text.substr(i + 1, text.size());
      }
    }
  }
  throw std::runtime_error("Text did not contain enough lines");
}

std::string crop_llvm_header(const std::string& text) {
  // Target triple is the last line of the header, then there should be an empty
  // line.
  auto index = text.find("target triple");
  if (index == std::string::npos) {
    throw std::runtime_error(
        "Text did not contain the target triple information");
  }

  return crop_first_lines(text.substr(index), 2);
}
