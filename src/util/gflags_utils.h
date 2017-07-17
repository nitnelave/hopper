#pragma once

#include <gflags/gflags.h>

namespace gflags {

/// This class is a wrapper to apply RAII to make sure we shut down command
/// line flags.
class GFlagsWrapper {
 public:
  GFlagsWrapper(int* argc, char*** argv, bool keep_rest) {
    ParseCommandLineFlags(argc, argv, keep_rest);
  }

  ~GFlagsWrapper() { gflags::ShutDownCommandLineFlags(); }
};

}  // namespace gflags
