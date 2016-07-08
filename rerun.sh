#!/bin/sh

set -x
set -e

rm -f ./build/flang

cd build


ASAN_SYMBOLIZER_PATH=$(which llvm-symbolizer)
ASAN_OPTIONS=symbolize=1

make "CC='clang'" "CFLAGS=-std=c11 -g -O0 -fsanitize=integer -fsanitize=undefined -fsanitize=address -fno-omit-frame-pointer -DASAN=0"

cd ..

./build/flang ./test/codegen/math.fl

clang -std=c11 -lpthread -llibuv -D_GNU_SOURCE c_code.c
