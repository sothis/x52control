#!/bin/bash

find $1 -wholename "*.git*" -prune -o -wholename "*normalize.sh" -prune -o \
-type f \( -iname "*.c" -o -iname "*.h" -o -iname "*.sh" -o -iname "*.rule" \
-o -iname "Makefile" \) \
-print0 | xargs -0 file | grep -Z -i text | awk -F ':' '{printf "%s\n",$1}' |
while read sourcefile
do
  fn=$(basename "$sourcefile")
  echo "processing: $fn"
 # removing trailing whitespaces
  sed -i -e 's/[[:space:]]*$//' "$sourcefile"
 # normalize line endings
  dos2unix -p -o -e -d -a "$sourcefile"
 # normalize file mode
  chmod 0644 "$sourcefile"
done

