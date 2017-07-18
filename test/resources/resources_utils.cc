#include "resources/resources_utils.h"

#include <fstream>

bool starts_with(const std::string& haystack, const std::string& needle) {
  if (haystack.size() < needle.size()) return false;
  return std::equal(needle.begin(), needle.end(), haystack.begin());
}

bool ends_with(const std::string& haystack, const std::string& needle) {
  if (haystack.size() < needle.size()) return false;
  return std::equal(needle.begin(), needle.end(),
                    haystack.end() - needle.size());
}

std::string read_file(const std::string& filename) {
  std::ifstream in(filename, std::ios::in | std::ios::binary);
  if (in) {
    std::ostringstream contents;
    contents << in.rdbuf();
    in.close();
    return (contents.str());
  }
  throw(errno);
}
