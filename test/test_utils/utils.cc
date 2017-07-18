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
