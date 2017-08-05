#! /bin/sh

if [ "$#" -gt 1 ] || ([ "$#" -eq 1 ] && [ "$1" != "--check" ])
then
  echo "Usage: $0 [--check]"
  exit 0
fi

CURRENT_DIR=$(dirname "$0")
PROJECT_DIR=$(readlink -f "${CURRENT_DIR}/..")

FORMAT_CMD="clang-format -style=Google"

FIND_CMD="find ${PROJECT_DIR}/src/ ${PROJECT_DIR}/test/ -regex '.*\.[hc]\{1,2\}' -type f"

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
  for FILE in $(eval ${FIND_CMD}); do
      echo "Formatting ${FILE}..."
      $(eval ${FORMAT_CMD} -i "${FILE}")
  done
fi
