#!/bin/sh

set -e
set -x

cd src/libparser/grammar

# debug/dev
#flex --header-file=tokens.h --debug -o tokens.c tokens.l parser.h
#bison -t -d -o parser.c parser.y

bison -d -o parser.c parser.y
flex --header-file=tokens.h -o tokens.c tokens.l parser.h

cd ../../../
