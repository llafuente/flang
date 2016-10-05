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
    fprintf(stderr, "Usage: flan file.fl [-v] [-vv] [-nocore]\n");
    exit(2);
  }

  bool core = true;
  bool log_user_code = false;
  bool log_core_code = false;

  // append to the end atm. i'm lazy
  for (int i = 0; i < argc; ++i) {
    if (strcmp("-v", argv[i]) == 0) {
      log_user_code = true;
    }
    if (strcmp("-vv", argv[i]) == 0) {
      log_user_code = true;
      log_core_code = true;
    }
    if (strcmp("-nocore", argv[i]) == 0) {
      log_debug_level = 10;
      core = false;
    }
  }

  // initialize global state
  flang_init();
  ast_t* root;

  log_debug_level = log_user_code ? 10 : 0;
  root = psr_file(argv[1]);

  if (core) {
    log_debug_level = log_core_code ? 10 : 0;
    psr_attach_core(root);
    ast_parent(root);
  }

  // check if it's contains an error
  if (ast_print_error(root)) {
    exit(3);
  }

  // TODO this 'log' need to be split if possible...
  log_debug_level = log_user_code || log_core_code ? 10 : 0;
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
