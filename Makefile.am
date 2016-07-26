ACLOCAL_AMFLAGS = -I m4

COMPILER_SOURCES = src/flang.c \
                src/debug.c \
                ext/array.c \
                ext/hash.c \
                ext/pool.c


AM_CPPFLAGS = -I$(top_srcdir)/ext \
              -I$(top_srcdir)/src

include_HEADERS = flang/flang.h \
  flang/debug.h \
  flang/libast.h \
  flang/libparser.h \
  flang/libcg.h \
  flang/libts.h

LLVM_CC_FLAGS=`llvm-config --cflags`
# core executionengine mcjit interpreter analysis native bitwriter
LLVM_LINK_FLAGS=`llvm-config --cxxflags --ldflags --system-libs --libs`



bin_PROGRAMS = flang
flang_SOURCES = src/main.c
flang_SOURCES += $(COMPILER_SOURCES)

flang_CFLAGS = $(LLVM_CC_FLAGS)
flang_LDFLAGS = $(LLVM_LINK_FLAGS) -lstdc++ -Wl -L/usr/local/lib '-lstringc'
flang_LDADD = libfl_codegen.a libfl_ts.a libfl_parser.a libfl_ast.a $(LIBS)
flang_DEPENDENCIES = libfl_codegen.a libfl_ts.a libfl_parser.a libfl_ast.a


SOURCES_TEST = test/tasks.c \
               test/unit/hash-test.c \
               test/unit/typesystem-test.c \
               test/unit/parser-literals.c \
               test/unit/parser-expressions-test.c \
               test/unit/parser-variables-test.c \
               test/unit/parser-functions-test.c \
               test/unit/parser-types-test.c \
               test/unit/parser-if-test.c \
               test/unit/parser-loop-test.c \
               test/unit/parser-scopes-test.c \
               test/unit/codegen-expressions-test.c \
               test/unit/codegen-functions-test.c \
               test/codegen/codegen-tests.c

TESTS = tests
check_PROGRAMS = tests

tests_CFLAGS = $(LLVM_CC_FLAGS) -O1
tests_LDFLAGS = $(LLVM_LINK_FLAGS) -O1 -lstdc++ -lrt -L/usr/local/lib '-lstringc'
tests_LDADD = libfl_codegen.a libfl_ts.a libfl_parser.a libfl_ast.a $(LIBS)
tests_DEPENDENCIES = libfl_codegen.a libfl_ts.a libfl_parser.a libfl_ast.a

tests_SOURCES = test/run-tests.c
#tests_SOURCES = test/run-benchmarks.c
tests_SOURCES += $(SOURCES_TEST)
tests_SOURCES += $(COMPILER_SOURCES)


noinst_LIBRARIES = libfl_ast.a libfl_parser.a libfl_ts.a libfl_codegen.a

libfl_ast_a_SOURCES = src/libast/ast-mk.c \
                        src/libast/ast-modify.c \
                        src/libast/ast-alloc.c \
                        src/libast/ast-traverse.c \
                        src/libast/ast-dump.c \
                        src/libast/ast-query.c \
                        src/libast/ast-reduce.c \
                        src/libast/ast-error.c \
                        src/libast/ast-search.c \
                        src/libast/ast-static.c \
                        src/libast/ast-expand.c \
                        src/libast/ast-reverse.c \
                        src/libast/ast.c

libfl_parser_a_SOURCES = src/libparser/grammar/parser.c \
                        src/libparser/grammar/tokens.c \
                        src/libparser/libparser.c

libfl_ts_a_SOURCES = src/libts/type-utils.c \
                        src/libts/typesystem-inference.c \
                        src/libts/typesystem-promotion.c \
                        src/libts/typesystem-pass.c \
                        src/libts/type-dump.c \
                        src/libts/typesystem-alloc.c \
                        src/libts/typesystem-casting.c \
                        src/libts/typesystem.c \
                        src/libts/typesystem-register.c \
                        src/libts/implement.c

libfl_ts_a_LIBADD = libfl_ast.a

libfl_codegen_a_SOURCES = src/libcg/c_codegen.c
