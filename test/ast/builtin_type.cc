#include "ast/builtin_type.h"
#include "test_utils/utils.h"

TEST(BuiltinType, BuiltinRange) {
  const lexer::Range& r = ast::internals::builtin_range;
  auto zero_pos = lexer::Range::Position{0, 0};
  EXPECT_EQ(r.to_string(),
            lexer::Range("<builtin>", zero_pos, zero_pos).to_string());
}

TEST(BuiltinType, UnknownIntType) {
  auto expected_none = ast::types::int_type_to_width(nullptr);

  EXPECT_FALSE(expected_none.is_ok());
}

TEST(BuiltinType, IntTypeWidth) {
#define INT_WIDTHS(DO) \
  DO(8)                \
  DO(16)               \
  DO(32)               \
  DO(64)

#define CHECK_INT_TO_WIDTH(WIDTH)                                              \
  {                                                                            \
    auto maybe_width = ast::types::int_type_to_width(&ast::types::int##WIDTH); \
    EXPECT_TRUE(maybe_width.is_ok());                                          \
    EXPECT_EQ(ast::types::IntWidth::W_##WIDTH, maybe_width.value_or_die());    \
  }
  INT_WIDTHS(CHECK_INT_TO_WIDTH);
#undef CHECK_INT_TO_WIDTH

#define CHECK_WIDTH_TO_INT(WIDTH)                                   \
  {                                                                 \
    auto type = width_to_int_type(ast::types::IntWidth::W_##WIDTH); \
    EXPECT_EQ(&ast::types::int##WIDTH, type);                       \
  }
  INT_WIDTHS(CHECK_WIDTH_TO_INT);
#undef CHECK_WIDTH_TO_INT
}

TEST(BuiltinType, Name) {
  EXPECT_EQ(ast::types::void_type.id().to_string(), "Void");
  EXPECT_EQ(ast::types::boolean.id().to_string(), "Bool");
#define CHECK_NAME(WIDTH) \
  EXPECT_EQ(ast::types::int##WIDTH.id().to_string(), "Int" #WIDTH);
  INT_WIDTHS(CHECK_NAME);
#undef CHECK_NAME
}

#undef INT_WIDTHS
