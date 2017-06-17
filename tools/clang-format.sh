#! /bin/sh

if [ $# -gt 1 ] || ([ $# -eq 1 ] && [ $1 != "--check" ])
then
  echo "Usage: $0 [--check]"
  exit 0
fi

FORMAT_CMD="clang-format -style=Google"

if [ $# -eq 1 ]
then
  EXIT=0
  for f in $(find src/ test/ -regex '.*\.\(h\|cc\)' -type f)
  do
    DIFF=$($FORMAT_CMD "$f" | diff --color=always --context=3 "$f" -)
    if [ "$DIFF" != "" ]
    then
      echo "File $f is not correctly formatted:"
      echo "$DIFF"
      EXIT=1
    fi
  done
  exit $EXIT
else
  find src/ test/ -regex '.*\.\(h\|cc\)' -type f -exec $FORMAT_CMD -i {} \;
fi


