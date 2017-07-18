#pragma once

#include <functional>

#include "gtest/gtest.h"

namespace test {

using TestFunction = testing::AssertionResult (*)(const std::string&);

testing::AssertionResult walk_directory(const char* folder,
                                        const TestFunction& tester);

}  // namespace test
