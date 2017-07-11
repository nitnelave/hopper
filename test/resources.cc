#include <fstream>

#include "ast/module.h"
#include "gflags/gflags.h"
#include "gtest/gtest.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "pretty_printer/pretty_printer.h"
#include "test_utils/files.h"
#include "test_utils/lexing.h"
#include "test_utils/utils.h"

DEFINE_string(test_resource_folder, "", "Location of the test resources");

struct ExpectedError {
  std::string message;
  lexer::Range range;
};

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

bool parse_position_line(int lineno, const std::string& line,
                         lexer::Range& range) {
  if (!starts_with(line, "//")) return false;
  if (starts_with(line, "// at EOF")) {
    range.begin = {-1, 1};
    range.end = {-1, 1};
    return true;
  }
  unsigned int i = 2;
  while (i < line.size() && line[i] == ' ') ++i;
  if (i == line.size() || line[i] != '^') return false;
  int start = i;
  ++i;
  // Keep reading while it's a '^'.
  for (; i < line.size() && line[i] == '^'; ++i) continue;
  range.begin = {lineno - 1, start + 1};
  if (ends_with(line, "to EOF"))
    range.end = {-1, 1};
  else
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

std::vector<ExpectedError> parse_expected_errors(const std::string& filename) {
  std::ifstream input(filename);
  std::vector<ExpectedError> result;
  lexer::Range error_range{filename, {1, 1}, {1, 1}};
  std::string error_message;
  std::string line;
  int lineno = 0;
  bool seen_position = false;
  while (std::getline(input, line)) {
    ++lineno;
    if (seen_position) {
      if (parse_error_message(line, error_message)) {
        result.push_back({error_message, error_range});
      } else {
        std::stringstream ss;
        ss << "A position (^^^) line should be immediately followed by an "
              "\"ERROR:\" line\nWhile reading "
           << filename << '\n';
        throw std::runtime_error(ss.str().c_str());
      }
    } else if (parse_position_line(lineno, line, error_range)) {
      seen_position = true;
      continue;
    }
    seen_position = false;
  }
  for (auto& error : result) {
    // Replace EOF with the proper line number.
    if (error.range.begin.line == -1) error.range.begin.line = lineno + 1;
    if (error.range.end.line == -1) error.range.end.line = lineno + 1;
  }
  return result;
}

testing::AssertionResult test_lexer_resource(const std::string& filename) {
  auto expected_errors =
      MAP_VEC(parse_expected_errors(filename),
              lexer::LexError(__ARG__.message, __ARG__.range));
  if (expected_errors.size() > 1) {
    std::stringstream ss;
    ss << "You must have at most one position (^^^) and \"ERROR\" line in a "
          "resource file, you had "
       << expected_errors.size() << "\nWhile reading " << filename << '\n';
    throw std::runtime_error(ss.str().c_str());
  }
  auto result = lexer::file_to_tokens(filename);
  if (result.is_ok() && !expected_errors.empty())
    return testing::AssertionFailure() << "Expected error:\n"
                                       << expected_errors[0] << "\nGot success";
  if (!result.is_ok()) {
    if (expected_errors.empty())
      return testing::AssertionFailure() << "Expected success, got:\n"
                                         << result.error_or_die();
    if (result.error_or_die() != expected_errors[0])
      return testing::AssertionFailure() << "Expected:\n"
                                         << expected_errors[0] << "\nGot:\n"
                                         << result.error_or_die();
  }
  return testing::AssertionSuccess();
}

testing::AssertionResult test_parser_resource(const std::string& filename) {
  auto expected_errors =
      MAP_VEC(parse_expected_errors(filename),
              parser::ParseError(__ARG__.message, __ARG__.range));
  if (expected_errors.size() > 1) {
    std::stringstream ss;
    ss << "You must have at most one position (^^^) and \"ERROR\" line in a "
          "resource file, you had "
       << expected_errors.size() << "\nWhile reading " << filename << '\n';
    throw std::runtime_error(ss.str().c_str());
  }
  lexer::Lexer lex = lexer::from_file(filename);
  parser::Parser parser(&lex);
  auto result = parser.parse();
  if (result.is_ok() && !expected_errors.empty())
    return testing::AssertionFailure() << "Expected error:\n"
                                       << expected_errors[0] << "\nGot success";
  if (!result.is_ok()) {
    if (expected_errors.empty())
      return testing::AssertionFailure() << "Expected success, got:\n"
                                         << result.error_or_die();
    if (result.error_or_die() != expected_errors[0])
      return testing::AssertionFailure() << "Expected:\n"
                                         << expected_errors[0] << "\nGot:\n"
                                         << result.error_or_die();
  }
  return testing::AssertionSuccess();
}

Variant<testing::AssertionResult, std::string> get_pretty_printed_file(
    const std::string& filename) {
  lexer::Lexer lex = lexer::from_file(filename);
  parser::Parser parser(&lex);
  auto result = parser.parse();
  if (!result.is_ok())
    return testing::AssertionFailure() << "Error while parsing " << filename
                                       << result.to_string() << '\n';
  std::stringstream ss;
  ast::PrettyPrinterVisitor visitor(ss);
  result.value_or_die()->accept(visitor);
  return ss.str();
}

testing::AssertionResult test_pretty_printer(const std::string& filename) {
  if (ends_with(filename, ".ref")) return testing::AssertionSuccess();
  assert(ends_with(filename, ".gh"));
  std::string ref_filename =
      filename.substr(0, filename.size() - sizeof(".gh") + 1) + ".ref";
  auto first_pass = get_pretty_printed_file(filename);
  if (first_pass.is<testing::AssertionResult>())
    return first_pass.get_unchecked<testing::AssertionResult>();
  const std::string& first_pass_result =
      first_pass.get_unchecked<std::string>();
  auto ref_contents = read_file(ref_filename);
  if (ref_contents != first_pass_result)
    return testing::AssertionFailure() << "Expected:\n"
                                       << ref_contents << "\nGot:\n"
                                       << first_pass_result;

  auto second_pass = get_pretty_printed_file(ref_filename);
  if (second_pass.is<testing::AssertionResult>())
    return second_pass.get_unchecked<testing::AssertionResult>();
  const std::string& second_pass_result =
      second_pass.get_unchecked<std::string>();
  if (ref_contents != second_pass_result)
    return testing::AssertionFailure()
           << "PrettyPrinter not stable. Expected:\n"
           << ref_contents << "\nGot:\n"
           << second_pass_result;
  return testing::AssertionSuccess();
}

TEST(ResourcesTest, Lexer) {
  EXPECT_FALSE(FLAGS_test_resource_folder.empty());
  EXPECT_TRUE(test::walk_directory(
      (FLAGS_test_resource_folder + "/lexer").c_str(), test_lexer_resource));
}

TEST(ResourcesTest, Parser) {
  EXPECT_FALSE(FLAGS_test_resource_folder.empty());
  EXPECT_TRUE(test::walk_directory(
      (FLAGS_test_resource_folder + "/parser").c_str(), test_parser_resource));
}

TEST(ResourcesTest, PrettyPrinter) {
  EXPECT_FALSE(FLAGS_test_resource_folder.empty());
  EXPECT_TRUE(test::walk_directory(
      (FLAGS_test_resource_folder + "/pretty_printer").c_str(),
      test_pretty_printer));
}
