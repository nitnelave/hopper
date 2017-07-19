#include "util/logging.h"

#include <iostream>

DEFINE_bool(logtostderr, false, "If true, debug messages go to stderr");
DEFINE_int32(verbosity, 1, "Verbosity level, from 0 to 4");

// log(INFO) << "Just a joke";
// log_if(INFO, a > 3) << "Nothing much";

internals::Logger log(Severity s) { return log_if(s, true); }

internals::Logger log_if(Severity s, bool condition) {
  return internals::Logger(FLAGS_logtostderr ? std::cerr : std::cout,
                           s <= FLAGS_verbosity && condition);
}
