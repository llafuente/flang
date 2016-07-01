#!/bin/sh

set -e
set -x

cd parser/grammar

# debug/dev
#flex --header-file=tokens.h --debug -o tokens.c tokens.l parser.h
#bison -t -d -o parser.c parser.y

flex --header-file=tokens.h -o tokens.c tokens.l parser.h
bison -d -o parser.c parser.y

cd ../..
