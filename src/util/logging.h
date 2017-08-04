#pragma once

/// This file contains the logging utilities.
///
/// It is highly discourage to write directly to std::cerr or std::cout.
/// Instead, use the logging utilities.
///
/// Example:
/// log(INFO) << "Answer to the universe: " << 42;
/// log_if(WARNING, a > 9000) << "It's over 9000!";
/// CHECK(K >= 0) << "Oh, no! The temperature went under absolute 0!";

#include <ostream>

#include "gflags/gflags.h"

// LCOV_EXCL_START: logging is not tested

/// This flag controls the verbosity of the messages to be printed.
DECLARE_int32(verbosity);

/// This enum controls the verbosity setting at which a message will appear.
enum Severity {
  /// Something happened that was not supposed to. If this is printed, a bug
  /// report should be submitted.
  ERROR = 0,
  /// You're doing something dangerous or insecure, but it still should work.
  /// Don't blame us if it doesn't, though, you were warned.
  /// Default verbosity level.
  WARNING,
  /// High-level information, about the stages of compilation, that developpers
  /// should see.
  INFO,
  /// Detailed messages about the implementation.
  DEBUG,
};

namespace logging {

/// RAII printer. This object is a lightweight wrapper around an ostream.
class Logger {
 public:
  /// No copy
  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;
  /// Move is okay.
  Logger(Logger&&) noexcept = default;
  Logger& operator=(Logger&&) noexcept = default;
  /// Constructor.
  Logger(std::ostream& out, bool should_print, bool is_fatal = false)
      : should_print_(should_print), is_fatal_(is_fatal), out_(out) {}

  template <typename T>
  const Logger& print(const T& value) const {
    if (should_print_) out_ << value;
    return *this;
  }

  /// On destruction, print a new line, and exit (if relevant).
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

/// Handle logging flags. Has to be called after flag parsing, but before any
/// logging.
void initialize_logging();
}  // namespace logging

/// Print messages with the given Severity.
logging::Logger log(Severity s);
/// Print messages if the condition is fulfilled. The condition is evaluated
/// once before the messages, and is guaranteed to be evaluated.
logging::Logger log_if(Severity s, bool condition);
/// Dummy logging object that doesn't print anything.
logging::Logger no_log();

/// Log a message with Severity ERROR, and then exit the program. Prefer to use
/// the macro LOG_FATAL.
logging::Logger log_fatal();

/// Macro for better reporting of fatal errors.
#define LOG_FATAL log_fatal() << __FILE__ ":" << __LINE__ << ": Fatal error. "
/// Evaluate the condition, and if it is false, then print the message and exit
/// the program. Prefer to use the macro CHECK.
logging::Logger check_or_die(bool condition);

/// Macro for better reporting of failed checks.
#define CHECK(CONDITION)                                \
  check_or_die((CONDITION)) << __FILE__ ":" << __LINE__ \
                            << ": Check `" #CONDITION "' failed. "

/// Debug asserts. Only compiled in debug mode. In release mode, the condition
/// is not evaluated.
#ifndef NDEBUG
#define DCHECK(CONDITION) CHECK(CONDITION)
#else
#define DCHECK(CONDITION) no_log()
#endif
// LCOV_EXCL_STOP
