#pragma once

#include <gflags/gflags.h>

#include "util/logging.h"

DECLARE_bool(h);
DECLARE_bool(help);
DECLARE_string(helpmatch);

namespace gflags {

// LCOV_EXCL_START

/// This class is a wrapper to apply RAII to make sure we shut down command
/// line flags.
class GFlagsWrapper {
 public:
  GFlagsWrapper(int* argc, char*** argv, bool keep_rest) {
    ParseCommandLineNonHelpFlags(argc, argv, keep_rest);
    // Override --help to show only flags defined in gHopper.
    if (FLAGS_help || FLAGS_h) {
      FLAGS_help = false;
      // This value is a hack that depends on the fact that the build directory
      // is one folder under the root of the project.
      // If we need a nicer help message, we need to change library or fork
      // gflags.
      FLAGS_helpmatch = "../src/";
    }
    gflags::HandleCommandLineHelpFlags();
    initialize_logging();
  }

  ~GFlagsWrapper() { gflags::ShutDownCommandLineFlags(); }
};

// LCOV_EXCL_STOP

}  // namespace gflags
