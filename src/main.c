#include <stdio.h>

#include "flang/flang.h"
#include "flang/libast.h"
#include "flang/libparser.h"
#include "flang/debug.h"
#include "flang/libts.h"
#include "flang/libcg.h"

// edit codes
// 2 invalid input
// 3 parse error
// 4 typesystem error
// 5 faltal error
// 6 unexpected error

int main(int argc, char** argv) {
  // TODO 0 means REPL
  if (argc == 1) {
    fprintf(stderr, "Usage: flan file.fl [-v] [-nocore]\n");
    exit(2);
  }

  bool core = true;

  // append to the end atm. i'm lazy
  for (int i = 0; i < argc; ++i) {
    if (strcmp("-v", argv[i]) == 0) {
      log_debug_level = 10;
    }
    if (strcmp("-nocore", argv[i]) == 0) {
      log_debug_level = 10;
      core = false;
    }
  }

  flang_init();

  // debug single file
  ast_t* root;
  if (core) {
    root = psr_file_main(argv[1]);
  } else {
    root = psr_file(argv[1]);
  }
  if (ast_print_error(root)) {
    exit(3);
  }

  root = psr_ast_check(root);
  if (ast_print_error(root)) {
    exit(3);
  }

  root = typesystem(root);
  if (ast_print_error(root)) {
    exit(4);
  }

  // ast_dump_s(root);

  // print to std atm...
  fl_codegen(root);

  flang_exit(root);

  return 0;
}
