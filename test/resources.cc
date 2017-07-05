#include <fstream>

#include "gflags/gflags.h"
#include "gtest/gtest.h"
#include "lexer/lexer.h"
#include "test_utils/files.h"
#include "test_utils/lexing.h"

DEFINE_string(test_resource_folder, "", "Location of the test resources");

bool starts_with(const std::string& haystack, const std::string& needle) {
  if (haystack.size() < needle.size()) return false;
  return std::equal(needle.begin(), needle.end(), haystack.begin());
}

bool parse_position_line(int lineno, const std::string& line,
                         lexer::Range& range) {
  if (!starts_with(line, "//")) return false;
  unsigned int i = 2;
  while (i < line.size() && line[i] == ' ') ++i;
  if (i == line.size() || line[i] != '^') return false;
  int start = i;
  ++i;
  // Keep reading while it's a '^'.
  for (; i < line.size() && line[i] == '^'; ++i) continue;
  range.begin = {lineno - 1, start + 1};
  range.end = {lineno - 1, static_cast<int>(i)};
  return true;
}

bool parse_error_message(const std::string& line, std::string& message) {
  if (starts_with(line, "// ERROR: ") && line.size() > 10) {
    message = line.substr(10);  // After ERROR.
    return true;
  }
  return false;
}

std::vector<lexer::LexError> parse_expected_errors(
    const std::string& filename) {
  std::ifstream input(filename);
  std::vector<lexer::LexError> result;
  lexer::Range error_range{filename, {1, 1}, {1, 1}};
  std::string error_message;
  std::string line;
  int lineno = 0;
  bool seen_position = false;
  while (std::getline(input, line)) {
    ++lineno;
    if (seen_position) {
      if (parse_error_message(line, error_message)) {
        result.emplace_back(error_message, error_range);
      } else {
        std::stringstream ss;
        ss << "A position (^^^) line should be immediately followed by an "
              "\"ERROR:\" line\nWhile reading "
           << filename;
        throw std::runtime_error(ss.str().c_str());
      }
    } else if (parse_position_line(lineno, line, error_range)) {
      seen_position = true;
      continue;
    }
    seen_position = false;
  }
  return result;
}

testing::AssertionResult test_lexer_resource(const std::string& filename) {
  std::cout << "Opening test file " << filename << '\n';
  auto expected_errors = parse_expected_errors(filename);
  if (expected_errors.size() != 1) {
    std::stringstream ss;
    ss << "You must have exactly one position (^^^) and \"ERROR\" line in a "
          "resource file, you had "
       << expected_errors.size() << "\nWhile reading " << filename;
    throw std::runtime_error(ss.str().c_str());
  }
  const lexer::LexError& expected_error = expected_errors[0];
  auto result = lexer::file_to_tokens(filename);
  if (result.is_ok())
    return testing::AssertionFailure() << "Expected error:\n"
                                       << expected_error << "\nGot success";
  if (result.error_or_die() != expected_error)
    return testing::AssertionFailure() << "Expected:\n"
                                       << expected_error << "\nGot:\n"
                                       << result.error_or_die();
  return testing::AssertionSuccess();
}

TEST(ResourcesTest, Lexer) {
  EXPECT_FALSE(FLAGS_test_resource_folder.empty());
  EXPECT_TRUE(test::walk_directory(
      (FLAGS_test_resource_folder + "/lexer").c_str(), test_lexer_resource));
}
