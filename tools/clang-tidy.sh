#! /bin/sh

CURRENT_DIR=$(readlink -f $(dirname $0))
PROJECT_DIR=$(readlink -f ${CURRENT_DIR}/..)
BUILD=${PROJECT_DIR}/build

cd ${BUILD}
python2 ${CURRENT_DIR}/run-clang-tidy.py \
  -p $BUILD \
  -header-filter="\.\./(src|test)/.*"
