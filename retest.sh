#!/bin/sh

set -x
set -e

#always delete the test, so it will not run the previous compiled
rm -f ./build/tests

cd build

ASAN_SYMBOLIZER_PATH=$(which llvm-symbolizer)
ASAN_OPTIONS=symbolize=1

set +e

# -fsanitize-memory-track-origins -fsanitize-memory could be needed ?
make check "CC='clang'" "CFLAGS=${LLVM_CC_FLAGS} -g -O0 -fsanitize=integer -fsanitize=undefined -fsanitize=address -fno-omit-frame-pointer -DASAN=1 -DYYDEBUG=1"

cd ..

/usr/bin/time -v ./build/tests

#./flang ../test/fl/pointers.fl
#./flang ../test/fl/fibonacci.fl
