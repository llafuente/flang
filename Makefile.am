ACLOCAL_AMFLAGS = -I m4

COMPILER_SOURCES = src/tokenizer/tokenizer.c \
                src/tokenizer/tokens.c \
                src/tokenizer/tokens-dump.c \
                src/parser/parser.c \
                src/parser/parser-dump.c \
                src/parser/parser-utils.c \
                src/parser/parser-stack.c \
                src/parser/parser-block.c \
                src/parser/parser-literal.c \
                src/parser/parser-expression.c \
                src/parser/parser-variable.c \
                src/parser/parser-function.c \
                src/parser/parser-types.c \
                src/parser/parser-comment.c \
                src/parser/parser-if.c \
                src/parser/parser-loops.c \
                src/parser/parser-load.c \
                src/ast/ast.c \
                src/ast/ast-alloc.c \
                src/ast/ast-dump.c \
                src/ast/ast-traverse.c \
                src/ast/ast-reverse.c \
                src/ast/ast-static.c \
                src/ast/ast-error.c \
                src/ast/ast-codegen.c \
                src/ast/ast-search.c \
                src/codegen/codegen.c \
                src/codegen/codegen-export.c \
                src/codegen/codegen-interpreter.c \
                src/codegen/codegen-utils.c \
                src/codegen/types.c \
                src/typesystem/pass-inference.c \
                src/typesystem/typesystem.c \
                src/typesystem/type-dump.c \
                src/typesystem/type-cast.c \
                src/debug.c \
                ext/array.c

AM_CPPFLAGS = -I$(top_srcdir)/include \
              -I$(top_srcdir)/ext \
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
               test/typesystem-test.c \
               test/parser-utils-test.c \
               test/parser-literals.c \
               test/parser-expressions-test.c \
               test/parser-variables-test.c \
               test/parser-functions-test.c \
               test/parser-types-test.c \
               test/parser-if-test.c \
               test/parser-loop-test.c \
               test/codegen-expressions-test.c \
               test/codegen-functions-test.c \
               test/flang-files-test.c

TESTS = test/run-tests
check_PROGRAMS = test/run-tests

test_run_tests_CFLAGS = $(LLVM_CC_FLAGS) -O1
test_run_tests_LDFLAGS = $(LLVM_LINK_FLAGS) -O1 -lstdc++ -lrt -L/usr/local/lib '-lstringc'

test_run_tests_SOURCES = test/run-tests.c
#test_run_tests_SOURCES = test/run-benchmarks.c
test_run_tests_SOURCES += $(SOURCES_TEST)
test_run_tests_SOURCES += $(COMPILER_SOURCES)
