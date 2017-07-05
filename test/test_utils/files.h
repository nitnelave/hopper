#pragma once

#include <functional>

#include "gtest/gtest.h"

namespace test {

testing::AssertionResult walk_directory(
    const char* folder,
    const std::function<testing::AssertionResult(const std::string&)>& tester);

}  // namespace test
