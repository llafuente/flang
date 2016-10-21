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

rm -f ./build/flang
rm -f ./codegen/run.c

cd build


ASAN_SYMBOLIZER_PATH=$(which llvm-symbolizer)
ASAN_OPTIONS=symbolize=1

if [ "$COMPILER" = "clang" ]; then
  make "CC='clang'" "CFLAGS=-g -O0 -fsanitize=integer -fsanitize=undefined -fsanitize=address -fno-omit-frame-pointer -DASAN=0"
else
  make "CC='gcc'" "CFLAGS=-g -O0 -fsanitize=address -fno-omit-frame-pointer -DASAN=0 -static-libasan -static-libubsan -lubsan"
fi

cd ..

#/usr/bin/time -v ./build/flang ./test/codegen/functions/templates.fl
#/usr/bin/time -v ./build/flang ./test/codegen/typesystem/struct-templates.fl
#/usr/bin/time -v ./build/flang ./test/codegen/typesystem/autocast.fl
#/usr/bin/time -v ./build/flang ./test/test.fl
#/usr/bin/time -v ./build/flang ./test/codegen/math/arithmetic.fl
#/usr/bin/time -v ./build/flang ./test/codegen/typesystem/struct-alias.fl
#/usr/bin/time -v ./build/flang ./test/codegen/typesystem/operator-overloading.fl

#/usr/bin/time -v ./build/flang ./test/codegen/typesystem/references.fl
#/usr/bin/time -v ./build/flang ./test/codegen/misc/log.fl
#/usr/bin/time -v ./build/flang ./test/codegen/typesystem/struct-properties.fl -v -vv
#/usr/bin/time -v ./build/flang ./test/codegen/typesystem/struct-templates2.fl
#/usr/bin/time -v ./build/flang ./test.fl
#/usr/bin/time -v ./build/flang ./lib/core/array.fl -v -nocore
#/usr/bin/time -v ./build/flang ./test/codegen/typesystem/operator-overloading3.fl -v
#/usr/bin/time -v ./build/flang ./test/codegen/typesystem/references2.fl -v
#/usr/bin/time -v ./build/flang ./test/codegen/misc/globals.fl -v
#/usr/bin/time -v ./build/flang ./test/codegen/array/array.fl -v
#/usr/bin/time -v ./build/flang ./test/codegen/typesystem/sizeof.implement.fl -v -nocore
#/usr/bin/time -v ./build/flang ./test/codegen/typesystem/templatesv2.fl -v -nocore
/usr/bin/time -v ./build/flang ./test/codegen/typesystem/struct-templates.fl -v

# compile c code
if [ "$COMPILER" = "clang" ]; then
  clang -std=c11 -Wno-parentheses-equality -lpthread -luv -lstringc -D_GNU_SOURCE codegen/run.c -o codegen/app
else
  gcc -std=c11 -Wno-parentheses-equality -D_GNU_SOURCE codegen/run.c -o codegen/app -pthread /usr/local/lib/libuv.a /usr/local/lib/libstringc.a
fi

# run it!
./codegen/app
