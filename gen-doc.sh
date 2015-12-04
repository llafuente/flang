#!/bin/bash

# execute with bash not sh!

reset
LLVM_CC_FLAGS=`llvm-config --cflags`
# create some symlinks because cldoc is very lazy...
rm -rf docs/xml
rm doc.md
rm ext/ext
rm include/ext
rm include/grammar
ln -sf ../ext include/ext
ln -sf ./ ext/ext
ln -sf ../src/grammar include/grammar

#cldoc inspect -DASAN=0 -DDOC=1 ${LLVM_CC_FLAGS} -- include/flang.h > inspect.html
cldoc generate -DASAN=0 -DDOC=1 ${LLVM_CC_FLAGS} -- --basedir=../ --language=c --output=docs/ --report include/flang.h
#cldoc generate -- --output doc/ --report include/test.h
node docs/cldoc2md.js docs/xml/*.xml > doc.md
#cat doc.md
#cat doc/xml/string.xml
rm ext/ext
rm include/ext
rm include/grammar
