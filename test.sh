#!/bin/sh -x

mkdir -p build
cd build

#always delete the test, so it will not run the previous compiled
rm -f ./test/run-tests

ASAN_SYMBOLIZER_PATH=$(which llvm-symbolizer)
ASAN_OPTIONS=symbolize=1

if [ "$1" = "--clean" ]
  then

    make clean
    make distclean
    sh ../autogen.sh
    ../configure
fi

# -fsanitize-memory-track-origins -fsanitize-memory could be needed ?
make check "CC='clang'" "CFLAGS=${LLVM_CC_FLAGS} -g -O0 -fsanitize=integer -fsanitize=undefined -fsanitize=address -fno-omit-frame-pointer"

if [ "$1" = "--run" ]
  then
  ./test/run-tests
fi

  #./flang ../test/fl/pointers.fl
  #./flang ../test/fl/fibonacci.fl
cd ..
