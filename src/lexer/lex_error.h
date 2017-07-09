#pragma once

#include "error/error.h"
#include "lexer/token.h"

namespace lexer {
class LexError : public GenericError {
 public:
  explicit LexError(const std::string& message, const Range& r)
      : GenericError(message + " in " + r.to_string()) {}
  ~LexError() override = default;
};

inline std::ostream& operator<<(std::ostream& os, const LexError& error) {
  return os << error.to_string();
}
}  // namespace lexer
