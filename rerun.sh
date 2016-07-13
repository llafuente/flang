#!/bin/sh

set -x
set -e

rm -f ./build/flang

cd build


ASAN_SYMBOLIZER_PATH=$(which llvm-symbolizer)
ASAN_OPTIONS=symbolize=1

make "CC='clang'" "CFLAGS=-std=c11 -g -O0 -fsanitize=integer -fsanitize=undefined -fsanitize=address -fno-omit-frame-pointer -DASAN=0"

cd ..

/usr/bin/time -v ./build/flang ./test/codegen/misc/hello-world.fl

clang -std=c11 -Wno-parentheses-equality -lpthread -luv -lstringc -D_GNU_SOURCE codegen/run.c -o codegen/app

./codegen/app
