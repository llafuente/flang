#!/bin/sh

#temporary
node tokens.js

mkdir build
cd build

ASAN_SYMBOLIZER_PATH=$(which llvm-symbolizer)
ASAN_OPTIONS=symbolize=1

make clean
make distclean
sh ../autogen.sh
../configure
# -fsanitize-memory-track-origins -fsanitize-memory could be needed ?
make "CC='clang'" "CFLAGS=-g -O0 -fsanitize=integer -fsanitize=undefined -fsanitize=address -fno-omit-frame-pointer"

cd ..

./build/test ./test/fl/hello-world.fl
