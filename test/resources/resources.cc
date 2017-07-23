#include <fstream>

#include "ast/module.h"
#include "codegen/codegen.h"
#include "gflags/gflags.h"
#include "gtest/gtest.h"
#include "lexer/lexer.h"
#include "name_resolution/visitor.h"
#include "parser/parser.h"
#include "pretty_printer/pretty_printer.h"
#include "resources/resources_utils.h"
#include "test_utils/files.h"
#include "test_utils/lexing.h"
#include "test_utils/utils.h"
#include "transform/function_value_body.h"
#include "visitor/error_visitor.h"

DEFINE_string(test_resource_folder, "", "Location of the test resources");

namespace test {

using ::testing::AssertionSuccess;
using ::testing::AssertionResult;
using ::testing::AssertionFailure;

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

AssertionResult test_lexer_resource(const std::string& filename) {
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
    return AssertionFailure() << "Expected error:\n"
                              << expected_errors[0] << "\nGot success";
  if (!result.is_ok()) {
    if (expected_errors.empty())
      return AssertionFailure() << "Expected success, got:\n"
                                << result.error_or_die();
    if (result.error_or_die() != expected_errors[0])
      return AssertionFailure() << "Expected:\n"
                                << expected_errors[0] << "\nGot:\n"
                                << result.error_or_die();
  }
  return AssertionSuccess();
}

AssertionResult test_parser_resource(const std::string& filename) {
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
    return AssertionFailure() << "Expected error:\n"
                              << expected_errors[0] << "\nGot success";
  if (!result.is_ok()) {
    if (expected_errors.empty())
      return AssertionFailure() << "Expected success, got:\n"
                                << result.error_or_die();
    if (result.error_or_die() != expected_errors[0])
      return AssertionFailure() << "Expected:\n"
                                << expected_errors[0] << "\nGot:\n"
                                << result.error_or_die();
  }
  return AssertionSuccess();
}

Variant<AssertionResult, std::string> get_pretty_printed_file(
    const std::string& filename) {
  lexer::Lexer lex = lexer::from_file(filename);
  parser::Parser parser(&lex);
  auto result = parser.parse();
  if (!result.is_ok())
    return AssertionFailure() << "Error while parsing " << filename
                              << result.to_string() << '\n';
  std::stringstream ss;
  ast::PrettyPrinterVisitor visitor(ss);
  result.value_or_die()->accept(visitor);
  return ss.str();
}

using MaybeVisitorError = MaybeError<ast::ErrorList<>>;

namespace internals {

struct GeneralCaseTag {};
struct SpecialCaseTag : GeneralCaseTag {};

template <typename Transformer,
          typename = decltype(std::declval<Transformer>().error_list())>
void apply_transformer(ast::Module* ast, MaybeVisitorError* status,
                       SpecialCaseTag /*unused*/) {
  if (!status->is_ok()) return;
  Transformer transformer;
  ast->accept(transformer);
  if (!transformer.error_list().errors().empty())
    *status = MaybeVisitorError(transformer.error_list());
}

template <typename Transformer>
void apply_transformer(ast::Module* ast, MaybeVisitorError* status,
                       GeneralCaseTag /*unused*/) {
  if (!status->is_ok()) return;
  Transformer transformer;
  ast->accept(transformer);
}
}  // namespace internals

template <typename... Transformer>
AssertionResult apply_all_transformers(ast::Module* ast) {
  MaybeVisitorError status;
  // Trick to run all the transformers, in order.
  using swallow = int[];
  (void)swallow{(internals::apply_transformer<Transformer>(
                     ast, &status, internals::SpecialCaseTag()),
                 0)...};
  if (!status.is_ok()) return AssertionFailure() << status.error_or_die();
  return AssertionSuccess();
}

template <typename... Transformer>
Variant<AssertionResult, std::string> get_transformed_pretty_printed_file(
    const std::string& filename) {
  lexer::Lexer lex = lexer::from_file(filename);
  parser::Parser parser(&lex);
  auto result = parser.parse();
  if (!result.is_ok())
    return AssertionFailure() << "Error while parsing " << filename
                              << result.to_string() << '\n';
  auto status =
      apply_all_transformers<Transformer...>(result.value_or_die().get());
  if (!status) return status;
  std::stringstream ss;
  ast::PrettyPrinterVisitor visitor(ss);
  result.value_or_die()->accept(visitor);
  return ss.str();
}

template <typename... Transformer>
Variant<AssertionResult, std::string> get_transformed_ir(
    const std::string& filename) {
  lexer::Lexer lex = lexer::from_file(filename);
  parser::Parser parser(&lex);
  auto result = parser.parse();
  if (!result.is_ok())
    return AssertionFailure() << "Error while parsing " << filename
                              << result.to_string() << '\n';

  auto status =
      apply_all_transformers<Transformer...>(result.value_or_die().get());
  if (!status) return status;
  std::string ir;
  codegen::LLVMInitializer llvm_init;
  codegen::CodeGenerator generator(filename);
  result.value_or_die()->accept(generator);
  llvm::raw_string_ostream out(ir);
  generator.print(out);
  return crop_first_lines(ir, 4);
}

AssertionResult test_pretty_printer(const std::string& filename) {
  if (ends_with(filename, ".ref")) return AssertionSuccess();
  assert(ends_with(filename, ".gh"));
  auto first_pass = get_pretty_printed_file(filename);
  if (first_pass.is<AssertionResult>())
    return first_pass.get_unchecked<AssertionResult>();
  const std::string& first_pass_result =
      first_pass.get_unchecked<std::string>();

  std::string ref_filename =
      filename.substr(0, filename.size() - sizeof(".gh") + 1) + ".ref";
  auto ref_contents = read_file(ref_filename);
  if (ref_contents != first_pass_result)
    return AssertionFailure() << "Expected:\n"
                              << ref_contents << "\nGot:\n"
                              << first_pass_result;

  auto second_pass = get_pretty_printed_file(ref_filename);
  if (second_pass.is<AssertionResult>())
    return second_pass.get_unchecked<AssertionResult>();
  const std::string& second_pass_result =
      second_pass.get_unchecked<std::string>();
  if (ref_contents != second_pass_result)
    return AssertionFailure() << "PrettyPrinter not stable. Expected:\n"
                              << ref_contents << "\nGot:\n"
                              << second_pass_result;
  return AssertionSuccess();
}

using TransformFunction =
    Variant<AssertionResult, std::string> (*)(const std::string&);

template <TransformFunction transform>
AssertionResult transformer_test(const std::string& filename) {
  if (ends_with(filename, ".ref")) return AssertionSuccess();
  assert(ends_with(filename, ".gh"));
  std::string ref_filename =
      filename.substr(0, filename.size() - sizeof(".gh") + 1) + ".ref";
  Variant<AssertionResult, std::string> first_pass = transform(filename);
  auto expected_errors = parse_expected_errors(filename);
  if (expected_errors.empty()) {
    if (first_pass.is<AssertionResult>())
      return first_pass.get_unchecked<AssertionResult>();
    const std::string& first_pass_result =
        first_pass.get_unchecked<std::string>();
    auto ref_contents = read_file(ref_filename);
    if (ref_contents != first_pass_result)
      return AssertionFailure() << "Expected:\n"
                                << ref_contents << "\nGot:\n"
                                << first_pass_result;
  } else {
    if (!first_pass.is<AssertionResult>())
      return AssertionFailure() << "Test passed, expected failure";
    auto actual_message =
        first_pass.get_unchecked<AssertionResult>().failure_message();
    std::stringstream ss;
    for (const auto& err : expected_errors) {
      ss << err.message << " in " << err.range.to_string();
    }
    return AssertionResult(ss.str() == actual_message)
           << "Expected: " + ss.str() + "\nGot:\n" + actual_message;
  }
  return AssertionSuccess();
}

template <TestFunction tester>
AssertionResult test_all_files_in_dir(const std::string& directory) {
  if (FLAGS_test_resource_folder.empty())
    return AssertionFailure() << "Could not find test resources\n"
                                 "Please give --test_resource_folder flag";
  return test::walk_directory(
      (FLAGS_test_resource_folder + "/" + directory).c_str(), tester);
}

TEST(ResourcesTest, Lexer) {
  EXPECT_TRUE(test_all_files_in_dir<test_lexer_resource>("lexer"));
}

TEST(ResourcesTest, Parser) {
  EXPECT_TRUE(test_all_files_in_dir<test_parser_resource>("parser"));
}

TEST(ResourcesTest, PrettyPrinter) {
  EXPECT_TRUE(test_all_files_in_dir<test_pretty_printer>("pretty_printer"));
}

TEST(ResourcesTest, FunctionValueBodyTransformer) {
  EXPECT_TRUE(test_all_files_in_dir<
              transformer_test<get_transformed_pretty_printed_file<
                  transform::FunctionValueBodyTransformer>>>(
      "transformer/function_value_body"));
}

TEST(ResourcesTest, CodeGenerator) {
  EXPECT_TRUE(
      test_all_files_in_dir<transformer_test<
          get_transformed_ir<transform::FunctionValueBodyTransformer>>>("ir"));
}

TEST(ResourcesTest, NameResolver) {
  EXPECT_TRUE(
      test_all_files_in_dir<transformer_test<
          get_transformed_pretty_printed_file<name_resolution::NameResolver>>>(
          "name_resolution"));
}

}  // namespace test
