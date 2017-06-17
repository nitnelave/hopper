#! /bin/sh

CURRENT_DIR=$(dirname $0)
PROJECT_DIR=$(readlink -f ${CURRENT_DIR}/..)
BUILD=${PROJECT_DIR}/build

python2 ${CURRENT_DIR}/run-clang-tidy.py \
  -p $BUILD \
  -header-filter="${PROJECT_DIR}/(src|test)"
