#!/bin/sh

set -x
set -e

COMPILER="clang"

for i in "$@"
do
case $i in
  --gcc)
    COMPILER="gcc"
  ;;
  --clang)
    COMPILER="clang"
  ;;
  *)
    # unknown option
  ;;
esac
done

#always delete the test, so it will not run the previous compiled
rm -f ./build/tests

cd build

ASAN_SYMBOLIZER_PATH=$(which llvm-symbolizer)
ASAN_OPTIONS=symbolize=1

set +e

# -fsanitize-memory-track-origins -fsanitize-memory could be needed ?
if [ "$COMPILER" == "clang" ]; then
  make check "CC='clang'" "CFLAGS=-g -O0 -fsanitize=integer -fsanitize=undefined -fsanitize=address -fno-omit-frame-pointer -DASAN=1 -DYYDEBUG=1"
else
  make check "CC='gcc'" "CFLAGS=-g -O0 -fsanitize=address -fno-omit-frame-pointer -DASAN=1 -DYYDEBUG=1 -static-libasan -static-libubsan -lubsan"
fi

cd ..

/usr/bin/time -v ./build/tests
