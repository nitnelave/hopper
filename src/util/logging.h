#pragma once

#include <ostream>

#include "gflags/gflags.h"

DECLARE_int32(verbosity);

enum Severity {
  ERROR = 1,
  WARNING,
  INFO,
  DEBUG,
};

namespace internals {

class Logger {
 public:
  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;
  Logger(Logger&&) = default;
  Logger& operator=(Logger&&) = default;
  Logger(std::ostream& out, bool should_print)
      : should_print_(should_print), out_(out) {}

  template <typename T>
  const Logger& print(const T& value) const {
#ifndef NDEBUG
    if (should_print_) out_ << value;
#endif
    return *this;
  }

  ~Logger() { print('\n'); }

 private:
  bool should_print_;
  std::ostream& out_;
};

template <typename T>
const Logger& operator<<(const Logger& out, const T& value) {
  return out.print(value);
}
}  // namespace internals

internals::Logger log(Severity s);
internals::Logger log_if(Severity s, bool condition);
