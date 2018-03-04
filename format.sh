#!/bin/sh
set -e
format=${CLANG_FORMAT:-clang-format}
find example src \( ! -name "RestBody.cpp" -a -name "*.[ch]pp" -o -name "*.hxx" \) \
     -exec $format -i --style=file {} \;
