#!/bin/sh

for file in $(find ./src/ -iname '*.c'); do
    echo "formatting file ${file}"

    clang-format -style=file ${file} > xx
    mv xx ${file}
done

for file in $(find ./include/ -iname '*.h'); do
    echo "formatting file ${file}"

    clang-format -style=file ${file} > xx
    mv xx ${file}
done

for file in $(find ./test/ -iname '*.c'); do
  echo "formatting file ${file}"

  clang-format -style=file ${file} > xx
  mv xx ${file}
done

for file in $(find ./test/ -iname '*.h'); do
  echo "formatting file ${file}"

  clang-format -style=file ${file} > xx
  mv xx ${file}
done

for file in "./ext/hash.h" "./ext/hash.c" "./ext/array.h" "./ext/array.c" "./ext/pool.h" "./ext/pool.c";
do
  echo "formatting file ${file}"

  clang-format -style=file ${file} > xx
  mv xx ${file}
done
