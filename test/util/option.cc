#include <sstream>
#include <string>

#include "gtest/gtest.h"

#include "util/option.h"

TEST(OptionVariantTest, Instantiate) {
  using VariantInt = Variant<int, std::string>;
  Option<VariantInt> op1 = none;
  EXPECT_FALSE(op1.is_ok());
  op1 = 3;
  EXPECT_TRUE(op1.is_ok());
  EXPECT_EQ(3, op1.get<int>());
  op1 = "test";
  EXPECT_TRUE(op1.is_ok());
  EXPECT_EQ("test", op1.get<std::string>());
  {
    Option<VariantInt> op2 = op1;
    EXPECT_TRUE(op1.is_ok());
    EXPECT_EQ("test", op1.get<std::string>());
    op1 = op2;
    EXPECT_TRUE(op1.is_ok());
    EXPECT_EQ("test", op1.get<std::string>());
    op2 = none;
    EXPECT_FALSE(op2.is_ok());
    op1 = std::move(op2);
    EXPECT_FALSE(op1.is_ok());
  }
  VariantInt v2(3);
  op1 = v2;
  EXPECT_TRUE(op1.is_ok());
  EXPECT_EQ(3, op1.get<int>());
  EXPECT_EQ(3, op1.value_or_die().get<int>());
  const auto& op3 = op1;
  EXPECT_EQ(3, op3.value_or_die().get<int>());
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
