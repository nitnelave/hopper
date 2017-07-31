#! /bin/sh

if [ "$#" -gt 1 ] || ([ "$#" -eq 1 ] && [ "$1" != "--check" ])
then
  echo "Usage: $0 [--check]"
  exit 0
fi

CURRENT_DIR=$(dirname "$0")
PROJECT_DIR=${CURRENT_DIR}/..

FORMAT_CMD="clang-format -style=Google"

FIND_CMD="find ${PROJECT_DIR}/src/ ${PROJECT_DIR}/test/ -name '*.cc' -or -name '*.h'  -type f"

if [ "$#" -eq 1 ]
then
  EXIT=0
  for f in $(eval "${FIND_CMD}")
  do
    DIFF=$($FORMAT_CMD "$f" | diff --context=3 "$f" -)
    if [ "$DIFF" != "" ]
    then
      echo "File $f is not correctly formatted:"
      echo "$DIFF"
      EXIT=1
    fi
  done
  exit $EXIT
else
  eval "${FIND_CMD} -exec $FORMAT_CMD -i {} \;"
fi


