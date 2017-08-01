#include "ast/builtin_type.h"
#include "test_utils/utils.h"

TEST(BuiltinType, BuiltinRange) {
  const lexer::Range& r = ast::internals::builtin_range;
  auto zero_pos = lexer::Range::Position{0, 0};
  EXPECT_EQ(r.to_string(),
            lexer::Range("<builtin>", zero_pos, zero_pos).to_string());
}

TEST(BuiltinType, IntTypeWidth) {
  using namespace ast::types;

#define INT_WIDTHS(DO) \
  DO(8)                \
  DO(16)               \
  DO(32)               \
  DO(64)

#define CHECK_INT_TO_WIDTH(WIDTH)                               \
  {                                                             \
    auto maybe_width = int_type_to_width(&int##WIDTH);          \
    EXPECT_TRUE(maybe_width.is_ok());                           \
    EXPECT_EQ(IntWidth::W_##WIDTH, maybe_width.value_or_die()); \
  }
  INT_WIDTHS(CHECK_INT_TO_WIDTH);
#undef CHECK_INT_TO_WIDTH

#define CHECK_WIDTH_TO_INT(WIDTH)                       \
  {                                                     \
    auto type = width_to_int_type(IntWidth::W_##WIDTH); \
    EXPECT_EQ(&int##WIDTH, type);                       \
  }
  INT_WIDTHS(CHECK_WIDTH_TO_INT);
#undef CHECK_WIDTH_TO_INT
}

TEST(BuiltinType, Name) {
  using namespace ast::types;
  EXPECT_EQ(void_type.id().to_string(), "Void");
  EXPECT_EQ(boolean.id().to_string(), "Bool");
#define CHECK_NAME(WIDTH) EXPECT_EQ(int##WIDTH.id().to_string(), "Int" #WIDTH);
  INT_WIDTHS(CHECK_NAME);
#undef CHECK_NAME
}

#undef INT_WIDTHS
