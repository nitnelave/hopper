#pragma once

#include "lexer/token.h"

namespace parser {
class Parser;

class ScopedLocation {
 public:
  explicit ScopedLocation(const Parser* parser);

  lexer::Range range() const;
  lexer::Range error_range() const;

 private:
  const Parser* parser_;
  const lexer::Range::Position position_;
};

}  // namespace parser
