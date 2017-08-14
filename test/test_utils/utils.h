#pragma once

#include <algorithm>
#include <type_traits>
#include <vector>

#include "gtest/gtest.h"

/// Run an assertion, and propagate the error if it failed.
#define CALL_ASSERT(CALL) \
  do {                    \
    auto res = (CALL);    \
    if (!res) return res; \
  } while (false)

/// Compare two values, and pretty-print the failure.
template <typename T1, typename T2>
testing::AssertionResult compare(const T1& expected, const T2& actual) {
  if (expected != actual)
    return testing::AssertionFailure()
           << "Expected " << testing::PrintToString(expected) << ", got "
           << testing::PrintToString(actual);
  return testing::AssertionSuccess();
}

namespace internals {
/// Return type of a function.
template <typename Function>
using result_of_t = typename std::result_of<Function>::type;
}  // namespace internals

/// Convert a vector to another of same length by applying the mapper function
/// on each element.
template <typename Mapper, typename In>
std::vector<internals::result_of_t<Mapper(In)>> convert_vector(
    const std::vector<In>& input, const Mapper& map) {
  std::vector<internals::result_of_t<Mapper(In)>> result;
  result.reserve(input.size());
  std::transform(input.begin(), input.end(), std::back_inserter(result), map);
  return result;
}

/// Convert a vector to another by simple mapping. A lambda is generated with
/// one argument called __ARG__ that can be used in the expression.
///
/// Ex: auto res = MAP_VEC(my_vector, 2 * __ARG__ + 1);
#define MAP_VEC(INPUT, LAMBDA) \
  convert_vector((INPUT), [](const auto& __ARG__) { return LAMBDA; })

std::string crop_first_lines(const std::string& text, int number_of_lines);
std::string crop_llvm_header(const std::string& text);
