#pragma once

#include <ostream>

#include "gflags/gflags.h"

DECLARE_int32(verbosity);

enum Severity {
  ERROR = 0,
  WARNING,
  INFO,
  DEBUG,
};

namespace internals {

class Logger {
 public:
  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;
  Logger(Logger&&) noexcept = default;
  Logger& operator=(Logger&&) noexcept = default;
  Logger(std::ostream& out, bool should_print, bool is_fatal = false)
      : should_print_(should_print), is_fatal_(is_fatal), out_(out) {}

  template <typename T>
  const Logger& print(const T& value) const {
#ifndef NDEBUG
    if (should_print_) out_ << value;
#endif
    return *this;
  }

  ~Logger() {
    print('\n');
    if (is_fatal_) std::exit(1);
  }

 private:
  bool should_print_;
  bool is_fatal_;
  std::ostream& out_;
};

template <typename T>
const Logger& operator<<(const Logger& out, const T& value) {
  return out.print(value);
}
}  // namespace internals

void initialize_logging();

internals::Logger log(Severity s);
internals::Logger log_if(Severity s, bool condition);
internals::Logger no_log();

internals::Logger log_fatal();

internals::Logger check_or_die(bool condition);

#define CHECK(CONDITION)                                \
  check_or_die((CONDITION)) << __FILE__ ":" << __LINE__ \
                            << ": Check `" #CONDITION "' failed. "

#ifndef NDEBUG
#define DCHECK(CONDITION) CHECK(CONDITION)
#else
#define DCHECK(CONDITION) no_log()
#endif
