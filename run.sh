#!/bin/sh

set -x
set -e

sh grammar.sh

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
make "CC='clang'" "CFLAGS=-std=c11 -g -O0 -fsanitize=integer -fsanitize=undefined -fsanitize=address -fno-omit-frame-pointer -DASAN=0"

cd ..

./build/flang ./test/codegen/math.fl
#./build/flang ./test/fl/arithmetic.fl
