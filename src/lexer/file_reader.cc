#include "lexer/file_reader.h"

#include <cstring>

namespace lexer {
auto FileReader::read_one_char() -> ErrorOr<State, LexError> {
  if (read_char_ == '\n') {
    ++read_loc_.line;
    read_loc_.column = 0;
  }
  ++read_loc_.column;
  if (!stream_->get(read_char_)) {
    // LCOV_EXCL_START: hard to provoke failure.
    if (stream_->eof())
      read_char_ = EOF;
    else if (stream_->fail())
      return LexError(std::strerror(errno), {read_loc_, read_loc_});
    // LCOV_EXCL_STOP
  }
  return State{read_char_, read_loc_};
}
}  // namespace lexer
