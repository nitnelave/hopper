#! /bin/sh

DIRNAME=$(dirname $(readlink -f "$0"))

ROOT_DIR=$(dirname "${DIRNAME}")

lcov --gcov-tool "${DIRNAME}/llvm-gcov" \
  --rc lcov_branch_coverage=1 \
  --rc lcov_excl_line="(^[-0-9: #]*};?$)|(LCOV_EXCL_LINE)|(assert\(false)" \
  --no-external \
  --directory "$ROOT_DIR" \
  $*
