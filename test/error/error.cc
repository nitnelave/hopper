#include <string>

#include "gtest/gtest.h"

#include "error/error.h"

// Tests of GenericError.
TEST(ErrorTest, ErrorMessage) {
  GenericError e("abc");
  EXPECT_EQ("abc", e.to_string());
}

// Tests of ErrorOr.
TEST(ErrorTest, ToStringValue) {
  ErrorOr<std::string, GenericError> res("abc");
  EXPECT_EQ("Ok", res.to_string());
}

TEST(ErrorTest, ToStringError) {
  ErrorOr<std::string, GenericError> res(GenericError("abc"));
  EXPECT_EQ("abc", res.to_string());
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
  EXPECT_EQ("Error", res.error_or_die().to_string());
}

TEST(ErrorTest, ConstructorErrorMove) {
  ErrorOr<std::string, GenericError> res(GenericError("Error"));
  ASSERT_FALSE(res.is_ok());
  EXPECT_EQ("Error", res.error_or_die().to_string());
}

class SpecificError : public GenericError {
 public:
  SpecificError() : GenericError("test") {}
  std::string to_string() const override { return m_; }

 private:
  std::string m_ = "success";
};

TEST(ErrorTest, TestObjectSlicing) {
  ErrorOr<std::string, SpecificError> tmp(SpecificError{});
  ErrorOr<std::string, GenericError> res(std::move(tmp));
  ASSERT_FALSE(res.is_ok());
  EXPECT_EQ("success", res.error_or_die().to_string());
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
  EXPECT_EQ("abc", res.error_or_die().to_string());
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
  EXPECT_EQ("abc", res.error_or_die().to_string());
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
  EXPECT_EQ("abc", res.error_or_die().to_string());
}

// Tests of macros (RETURN_IF_ERROR and RETURN_OR_ASSIGN).

ErrorOr<int> maybe_return_int(bool error) {
  if (error) return GenericError("No int");
  return 1;
}

ErrorOr<bool> maybe_handle_int_assign(bool error) {
  RETURN_OR_ASSIGN(int i, maybe_return_int(false));
  RETURN_OR_ASSIGN(int j, maybe_return_int(error));
  return i == j;
}

MaybeError<> maybe_handle_int_return(bool error) {
  RETURN_IF_ERROR(maybe_return_int(false));
  RETURN_IF_ERROR(maybe_return_int(error));
  return {};
}

TEST(ErrorTest, ReturnOrAssignReturn) {
  auto res = maybe_handle_int_assign(false);
  ASSERT_TRUE(res.is_ok());
  EXPECT_EQ(true, res.value_or_die());
}

TEST(ErrorTest, ReturnOrAssignAssign) {
  auto res = maybe_handle_int_assign(true);
  ASSERT_FALSE(res.is_ok());
  EXPECT_EQ("No int", res.error_or_die().to_string());
}

TEST(ErrorTest, ReturnIfErrorNoError) {
  auto res = maybe_handle_int_return(false);
  ASSERT_TRUE(res.is_ok());
}

TEST(ErrorTest, ReturnIfErrorError) {
  auto res = maybe_handle_int_return(true);
  ASSERT_FALSE(res.is_ok());
  EXPECT_EQ("No int", res.error_or_die().to_string());
}
