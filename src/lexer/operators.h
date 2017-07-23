#pragma once

#include <cassert>

#include "lexer/token.h"
#include "util/option.h"

namespace lexer {

#define BINARY_OPERATOR_ENUM(X)            \
  X(OR, 1, OR)                             \
  X(AND, 2, AND)                           \
  X(BITOR, 3, BITOR)                       \
  X(BITXOR, 4, BITXOR)                     \
  X(BITAND, 5, AMPERSAND)                  \
  X(EQUAL, 6, EQUAL)                       \
  X(DIFFERENT, 6, DIFFERENT)               \
  X(GREATER, 7, GREATER)                   \
  X(GREATER_OR_EQUAL, 7, GREATER_OR_EQUAL) \
  X(LESS, 8, LESS)                         \
  X(LESS_OR_EQUAL, 8, LESS_OR_EQUAL)       \
  X(BITSHIFT_RIGHT, 8, BITSHIFT_RIGHT)     \
  X(BITSHIFT_LEFT, 8, BITSHIFT_LEFT)       \
  X(PLUS, 9, PLUS)                         \
  X(MINUS, 9, MINUS)                       \
  X(TIMES, 10, STAR)                       \
  X(DIVIDE, 10, DIVIDE)                    \
  X(INT_DIVIDE, 10, DIV)                   \
  X(MODULO, 10, MODULO)                    \
  X(QUESTION_MARK_COLON, 10, QUESTION_MARK_COLON)

enum class BinaryOperator : int {
#define MAKE_ENUM(NAME, VALUE, TOKEN) NAME,
  BINARY_OPERATOR_ENUM(MAKE_ENUM)
#undef MAKE_ENUM
      __NUMBER_OPERATORS__
};

inline Option<BinaryOperator> token_to_binary_operator(TokenType tt) {
  switch (tt) {
#define MAKE_CASE(NAME, VALUE, TOKEN) \
  case TokenType::TOKEN:              \
    return BinaryOperator::NAME;
    BINARY_OPERATOR_ENUM(MAKE_CASE)
#undef MAKE_CASE
    default:
      return none;
  }
}

namespace internals {
const int binary_operator_precedence[] = {
#define MAKE_PRECEDENCE(NAME, VALUE, TOKEN) VALUE,
    BINARY_OPERATOR_ENUM(MAKE_PRECEDENCE)
#undef MAKE_PRECEDENCE
};

const TokenType binary_operator_token[] = {
#define MAKE_TOKEN(NAME, VALUE, TOKEN) TokenType::TOKEN,
    BINARY_OPERATOR_ENUM(MAKE_TOKEN)
#undef MAKE_TOKEN
};

#undef BINARY_OPERATOR_ENUM

inline int binop_to_index(BinaryOperator t) {
  int index = static_cast<int>(t);
  assert(index >= 0 &&
         index < static_cast<int>(BinaryOperator::__NUMBER_OPERATORS__));
  return index;
}
}  // namespace internals

inline int operator_precedence(BinaryOperator op) {
  return internals::binary_operator_precedence[internals::binop_to_index(op)];
}

inline TokenType operator_to_token(BinaryOperator op) {
  return internals::binary_operator_token[internals::binop_to_index(op)];
}

inline std::string to_string(BinaryOperator binop) {
  return to_symbol(operator_to_token(binop));
}

inline std::ostream& operator<<(std::ostream& os, BinaryOperator binop) {
  return os << to_string(binop);
}

}  // namespace lexer
