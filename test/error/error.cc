#include <string>

#include "gtest/gtest.h"

#include "error/error.h"

// Tests of GenericError.
TEST(ErrorTest, ErrorMessage) {
  GenericError e("abc");
  EXPECT_EQ("abc", e.toString());
}

// Tests of ErrorOr.
TEST(ErrorTest, ToStringValue) {
  ErrorOr<std::string, GenericError> res("abc");
  EXPECT_EQ("Ok", res.toString());
}

TEST(ErrorTest, ToStringError) {
  ErrorOr<std::string, GenericError> res(GenericError("abc"));
  EXPECT_EQ("abc", res.toString());
}

TEST(ErrorTest, ConstructorValueCopy) {
  std::string a = "abc";
  ErrorOr<std::string, GenericError> res(a);
  ASSERT_TRUE(res.is_ok());
  EXPECT_EQ(a, res.value_or_die());
}

TEST(ErrorTest, ConstructorValueMove) {
  ErrorOr<std::string, GenericError> res("abc");
  ASSERT_TRUE(res.is_ok());
  EXPECT_EQ("abc", res.value_or_die());
}

TEST(ErrorTest, ConstructorErrorCopy) {
  GenericError err("Error");
  ErrorOr<std::string, GenericError> res(err);
  ASSERT_FALSE(res.is_ok());
  EXPECT_EQ("Error", res.error_or_die().toString());
}

TEST(ErrorTest, ConstructorErrorMove) {
  ErrorOr<std::string, GenericError> res(GenericError("Error"));
  ASSERT_FALSE(res.is_ok());
  EXPECT_EQ("Error", res.error_or_die().toString());
}

TEST(ErrorTest, CopyConstructorValue) {
  ErrorOr<std::string, GenericError> tmp("abc");
  ErrorOr<std::string, GenericError> res(tmp);
  EXPECT_TRUE(res.is_ok());
  EXPECT_EQ(tmp.value_or_die(), res.value_or_die());
}

TEST(ErrorTest, CopyConstructorError) {
  ErrorOr<std::string, GenericError> tmp(GenericError("abc"));
  ErrorOr<std::string, GenericError> res(tmp);
  EXPECT_TRUE(!res.is_ok());
  EXPECT_EQ(tmp.error_or_die().toString(), res.error_or_die().toString());
}

TEST(ErrorTest, MoveConstructorValue) {
  ErrorOr<std::string, GenericError> tmp("abc");
  ErrorOr<std::string, GenericError> res(std::move(tmp));
  ASSERT_TRUE(res.is_ok());
  EXPECT_EQ("abc", res.value_or_die());
}

TEST(ErrorTest, MoveConstructorError) {
  ErrorOr<std::string, GenericError> tmp(GenericError("abc"));
  ErrorOr<std::string, GenericError> res(std::move(tmp));
  ASSERT_FALSE(res.is_ok());
  EXPECT_EQ("abc", res.error_or_die().toString());
}

TEST(ErrorTest, CopyAssignmentValueToValue) {
  ErrorOr<std::string, GenericError> tmp("abc");
  ErrorOr<std::string, GenericError> res("def");
  res = tmp;
  EXPECT_TRUE(res.is_ok());
  EXPECT_EQ(tmp.value_or_die(), res.value_or_die());
}

TEST(ErrorTest, CopyAssignmentValueToError) {
  ErrorOr<std::string, GenericError> tmp(GenericError("abc"));
  ErrorOr<std::string, GenericError> res("def");
  res = tmp;
  EXPECT_TRUE(!res.is_ok());
  EXPECT_EQ(tmp.error_or_die().toString(), res.error_or_die().toString());
}

TEST(ErrorTest, CopyAssignmentErrorToValue) {
  ErrorOr<std::string, GenericError> tmp("abc");
  ErrorOr<std::string, GenericError> res(GenericError("def"));
  res = tmp;
  EXPECT_TRUE(res.is_ok());
  EXPECT_EQ(tmp.value_or_die(), res.value_or_die());
}

TEST(ErrorTest, CopyAssignmentErrorToError) {
  ErrorOr<std::string, GenericError> tmp(GenericError("abc"));
  ErrorOr<std::string, GenericError> res(GenericError("def"));
  res = tmp;
  EXPECT_TRUE(!res.is_ok());
  EXPECT_EQ(tmp.error_or_die().toString(), res.error_or_die().toString());
}

TEST(ErrorTest, MoveAssignmentValueToValue) {
  ErrorOr<std::string, GenericError> tmp("abc");
  ErrorOr<std::string, GenericError> res("def");
  res = std::move(tmp);
  ASSERT_TRUE(res.is_ok());
  EXPECT_EQ("abc", res.value_or_die());
}

TEST(ErrorTest, MoveAssignmentValueToError) {
  ErrorOr<std::string, GenericError> tmp(GenericError("abc"));
  ErrorOr<std::string, GenericError> res("def");
  res = std::move(tmp);
  ASSERT_FALSE(res.is_ok());
  EXPECT_EQ("abc", res.error_or_die().toString());
}

TEST(ErrorTest, MoveAssignmentErrorToValue) {
  ErrorOr<std::string, GenericError> tmp("abc");
  ErrorOr<std::string, GenericError> res(GenericError("def"));
  res = std::move(tmp);
  ASSERT_TRUE(res.is_ok());
  EXPECT_EQ("abc", res.value_or_die());
}

TEST(ErrorTest, MoveAssignmentErrorToError) {
  ErrorOr<std::string, GenericError> tmp(GenericError("abc"));
  ErrorOr<std::string, GenericError> res(GenericError("def"));
  res = std::move(tmp);
  ASSERT_FALSE(res.is_ok());
  EXPECT_EQ("abc", res.error_or_die().toString());
}

// Tests of macros (RETURN_IF_ERROR and RETURN_OR_ASSIGN).

ErrorOr<int> maybeReturnInt(bool error) {
  if (error)
    return GenericError("No int");
  else
    return 1;
}

ErrorOr<bool> maybeHandleIntAssign(bool error) {
  RETURN_OR_ASSIGN(int i, maybeReturnInt(false));
  RETURN_OR_ASSIGN(int j, maybeReturnInt(error));
  return i == j;
}

MaybeError<> maybeHandleIntReturn(bool error) {
  RETURN_IF_ERROR(maybeReturnInt(false));
  RETURN_IF_ERROR(maybeReturnInt(error));
  return {};
}

TEST(ErrorTest, ReturnOrAssignReturn) {
  auto res = maybeHandleIntAssign(false);
  ASSERT_TRUE(res.is_ok());
  EXPECT_EQ(true, res.value_or_die());
}

TEST(ErrorTest, ReturnOrAssignAssign) {
  auto res = maybeHandleIntAssign(true);
  ASSERT_FALSE(res.is_ok());
  EXPECT_EQ("No int", res.error_or_die().toString());
}

TEST(ErrorTest, ReturnIfErrorNoError) {
  auto res = maybeHandleIntReturn(false);
  ASSERT_TRUE(res.is_ok());
}

TEST(ErrorTest, ReturnIfErrorError) {
  auto res = maybeHandleIntReturn(true);
  ASSERT_FALSE(res.is_ok());
  EXPECT_EQ("No int", res.error_or_die().toString());
}
