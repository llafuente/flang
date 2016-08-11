#!/bin/sh

set -x
set -e

sh grammar.sh

rm -rf tmp
mkdir -p tmp
rm -f ./build/tests

mkdir -p build
cd build

sh ../bootstrap
../configure

cd ..
sh retest.sh
