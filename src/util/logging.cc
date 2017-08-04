#include "util/logging.h"

#include <iostream>

// LCOV_EXCL_START: logging is not tested

DEFINE_bool(logtostderr, false, "If true, debug messages go to stderr");
DEFINE_int32(
    verbosity, WARNING,
    "Verbosity level, one of 0 (error), 1 (warning), 2 (info), 3 (debug)");
DEFINE_bool(quiet, false, "Set verbosity to error (0)");
DEFINE_bool(warnings, false, "Set verbosity to warning (1)");
DEFINE_bool(verbose, false, "Set verbosity to info (2)");
DEFINE_bool(debug, false, "Set verbosity to debug (3)");

using logging::Logger;

namespace {
bool validate_verbosity(const char* flag_name, gflags::int32 value) {
  if (value < ERROR || value > DEBUG) {
    std::cerr << "Invalid value for " << flag_name << ": " << value << '\n';
    return false;
  }
  return true;
}
}  // namespace

DEFINE_validator(verbosity, &validate_verbosity);

namespace logging {
void initialize_logging() {
  if (FLAGS_quiet) gflags::SetCommandLineOption("verbosity", "0");
  if (FLAGS_warnings) gflags::SetCommandLineOption("verbosity", "1");
  if (FLAGS_verbose) gflags::SetCommandLineOption("verbosity", "2");
  if (FLAGS_debug) gflags::SetCommandLineOption("verbosity", "3");
}
}  // namespace logging

Logger log(Severity s) { return log_if(s, true); }

Logger no_log() { return log_if(INFO, false); }

Logger log_if(Severity s, bool condition) {
  return Logger(FLAGS_logtostderr ? std::cerr : std::cout,
                           s <= FLAGS_verbosity && condition);
}

Logger log_fatal() {
  return Logger(std::cerr, true, true);
}

Logger check(bool condition) {
  // If condition is false, print messages and then crash.
  return Logger(std::cerr, !condition, !condition);
}
// LCOV_EXCL_STOP
