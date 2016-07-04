ACLOCAL_AMFLAGS = -I m4

COMPILER_SOURCES = src/flang.c \
                src/debug.c \
                ext/array.c \
                ext/hash.c \
                ext/pool.c \
                src/typesystem/type-utils.c \
                src/typesystem/typesystem-inference.c \
                src/typesystem/typesystem-promotion.c \
                src/typesystem/typesystem-pass.c \
                src/typesystem/type-dump.c \
                src/typesystem/typesystem-alloc.c \
                src/typesystem/typesystem-casting.c \
                src/typesystem/typesystem.c \
                src/typesystem/typesystem-register.c


AM_CPPFLAGS = -I$(top_srcdir)/include \
              -I$(top_srcdir)/ext \
              -I$(top_srcdir)/src

include_HEADERS = flang/ast.h \
  flang/common.h \
  flang/debug.h \
  flang/libast.h \
  flang/libparserfl.h \
  flang/typesystem.h

LLVM_CC_FLAGS=`llvm-config --cflags`
# core executionengine mcjit interpreter analysis native bitwriter
LLVM_LINK_FLAGS=`llvm-config --cxxflags --ldflags --system-libs --libs`



bin_PROGRAMS = flang
flang_SOURCES = src/main.c
flang_SOURCES += $(COMPILER_SOURCES)

flang_CFLAGS = $(LLVM_CC_FLAGS)
flang_LDFLAGS = $(LLVM_LINK_FLAGS) -lstdc++ -Wl -L/usr/local/lib '-lstringc'
flang_LDADD = libparser_fl.a libast_fl.a $(LIBS)
flang_DEPENDENCIES = libparser_fl.a libast_fl.a


SOURCES_TEST = test/tasks.c \
               test/hash-test.c \
               test/typesystem-test.c \
               test/parser-literals.c \
               test/parser-expressions-test.c \
               test/parser-variables-test.c \
               test/parser-functions-test.c \
               test/parser-types-test.c \
               test/parser-if-test.c \
               test/parser-loop-test.c \
               test/parser-scopes-test.c \
               test/codegen-expressions-test.c \
               test/codegen-functions-test.c

# test/flang-files-test.c

TESTS = test/run-tests
check_PROGRAMS = test/run-tests

test_run_tests_CFLAGS = $(LLVM_CC_FLAGS) -O1
test_run_tests_LDFLAGS = $(LLVM_LINK_FLAGS) -O1 -lstdc++ -lrt -L/usr/local/lib '-lstringc'

test_run_tests_SOURCES = test/run-tests.c
#test_run_tests_SOURCES = test/run-benchmarks.c
test_run_tests_SOURCES += $(SOURCES_TEST)
test_run_tests_SOURCES += $(COMPILER_SOURCES)



noinst_LIBRARIES = libast_fl.a libparser_fl.a
libparser_fl_a_SOURCES = parser/grammar/parser.c \
                        parser/grammar/tokens.c \
                        parser/libparserfl.c

libast_fl_a_SOURCES = ast/ast-mk.c \
                        ast/ast-modify.c \
                        ast/ast-get.c \
                        ast/ast-alloc.c \
                        ast/ast-traverse.c \
                        ast/ast-dump.c \
                        ast/ast-query.c \
                        ast/ast-reduce.c \
                        ast/ast-error.c \
                        ast/ast-search.c \
                        ast/ast-codegen.c \
                        ast/ast-load-imports.c \
                        ast/ast-static.c \
                        ast/ast-expand.c \
                        ast/ast-reverse.c \
                        ast/ast.c
