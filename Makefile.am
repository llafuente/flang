ACLOCAL_AMFLAGS = -I m4

COMPILER_SOURCES = src/tokenizer.c \
                src/tokens.c \
                src/parser.c \
                src/parser-utils.c \
                src/parser-stack.c \
                src/parser-literal.c \
                src/parser-expression.c \
                src/parser-variable.c \
                src/parser-type.c \
                src/codegen.c \
                src/ast.c

AM_CPPFLAGS = -I$(top_srcdir)/include \
              -I$(top_srcdir)/src

include_HEADERS = include/flang.h

LLVM_CC_FLAGS=`llvm-config --cflags`
# core executionengine mcjit interpreter analysis native bitwriter
LLVM_LINK_FLAGS=`llvm-config --cxxflags --ldflags --system-libs --libs`



bin_PROGRAMS = flang
flang_SOURCES = src/main.c
flang_SOURCES += $(COMPILER_SOURCES)

flang_CFLAGS = $(LLVM_CC_FLAGS)
flang_LDFLAGS = $(LLVM_LINK_FLAGS) -lstdc++ -Wl -L/usr/local/lib '-lstringc'
flang_LDADD =

SOURCES_TEST = test/tasks.c \
               test/tokenizer-test.c \
               test/parser-utils-test.c \
               test/parser-literals.c \
               test/parser-expressions-test.c \
               test/parser-variables-test.c \
               test/codegen-expressions-test.c

TESTS = test/run-tests
check_PROGRAMS = test/run-tests

test_run_tests_CFLAGS = $(LLVM_CC_FLAGS) -O1
test_run_tests_LDFLAGS = $(LLVM_LINK_FLAGS) -O1 -lstdc++ -lrt -L/usr/local/lib '-lstringc'

test_run_tests_SOURCES = test/run-tests.c
#test_run_tests_SOURCES = test/run-benchmarks.c
test_run_tests_SOURCES += $(SOURCES_TEST)
test_run_tests_SOURCES += $(COMPILER_SOURCES)
