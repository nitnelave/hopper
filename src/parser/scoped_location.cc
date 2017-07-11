#include "parser/scoped_location.h"

#include "parser/parser.h"

namespace parser {

ScopedLocation::ScopedLocation(const Parser* parser)
    : parser_(parser), position_(parser_->current_token().location().begin) {}

lexer::Range ScopedLocation::range() const {
  return {parser_->current_token().location().file, position_,
          parser_->last_end_};
}

lexer::Range ScopedLocation::error_range() const {
  return {parser_->current_token().location().file, position_,
          parser_->current_token().location().end};
}

}  // namespace parser
