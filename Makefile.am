ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = -I$(top_srcdir)/include \
              -I$(top_srcdir)/src

include_HEADERS = include/flang.h

bin_PROGRAMS = test
test_SOURCES = src/main.c \
               src/tokenizer.c \
               src/tokens.c
test_LDFLAGS = -L/usr/local/lib '-lstringc'
test_LDADD =
