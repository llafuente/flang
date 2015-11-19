#!/bin/sh -x

cd src/grammar

# debug/dev
#flex --header-file=tokens.h --debug -o tokens.c tokens.l parser.h
#bison --warnings=error rs -v --report=all -d -o parser.c parser.y

flex --header-file=tokens.h -o tokens.c tokens.l parser.h
bison -d -o parser.c parser.y

cd ../..
