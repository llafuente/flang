#!/bin/sh

set -x
set -e

rm -rf build
mkdir -p build
cd build

ASAN_SYMBOLIZER_PATH=$(which llvm-symbolizer)
ASAN_OPTIONS=symbolize=1

#make clean
#make distclean
sh ../bootstrap
../configure
# -fsanitize-memory-track-origins -fsanitize-memory could be needed ?
make "CC='clang'" "CFLAGS=-g -O0 -fsanitize=integer -fsanitize=undefined -fsanitize=address -fno-omit-frame-pointer -DASAN=0"

cd ..

./build/flang ./test/fl/hello-world.fl
