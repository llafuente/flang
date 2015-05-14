ACLOCAL_AMFLAGS = -I m4

COMPILER_SOURCES = src/tokenizer.c \
                src/tokens.c \
                src/parser.c \
                src/parser-utils.c \
                src/parser-stack.c \
                src/parser-literal.c \
                src/ast.c

AM_CPPFLAGS = -I$(top_srcdir)/include \
              -I$(top_srcdir)/src

include_HEADERS = include/flang.h

bin_PROGRAMS = flang
flang_SOURCES = src/main.c
flang_SOURCES += ${COMPILER_SOURCES}

flang_LDFLAGS = -L/usr/local/lib '-lstringc'
flang_LDADD =

SOURCES_TEST = test/tasks.c \
               test/tokenizer-test.c \
               test/parser-utils-test.c \
               test/parser-literals.c

TESTS = test/run-tests
check_PROGRAMS = test/run-tests
test_run_tests_CFLAGS = -O1
test_run_tests_LDFLAGS = -O1 -lrt -L/usr/local/lib '-lstringc'
test_run_tests_SOURCES = test/run-tests.c
#test_run_tests_SOURCES = test/run-benchmarks.c
test_run_tests_SOURCES += ${SOURCES_TEST}
test_run_tests_SOURCES += ${COMPILER_SOURCES}
