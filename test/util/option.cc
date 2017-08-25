#include <sstream>
#include <string>

#include "gtest/gtest.h"

#include "util/option.h"

TEST(OptionVariantTest, Instantiate) {
  using VariantInt = Variant<int, std::string>;
  Option<VariantInt> op1 = none;
  op1 = 3;
  op1 = "test";
  Option<VariantInt> op2 = op1;
  op1 = op2;
  op1 = std::move(op2);
}

TEST(OptionPointerTest, Instantiate) {
  Option<int*> op1 = none;
  static_assert(sizeof(op1) == sizeof(int*), "");
  EXPECT_FALSE(op1.is_ok());
  int i = 3;
  op1 = &i;
  EXPECT_TRUE(op1.is_ok());
  Option<int*> op2 = op1;
  op1 = op2;
  EXPECT_TRUE(op1.is_ok());
  op2 = none;
  op1 = std::move(op2);
  EXPECT_FALSE(op1.is_ok());
}
