#include <stdio.h>
#include "flang.h"

array* identifiers;

int main(int argc, char** argv) {
  // TODO 0 means REPL
  if (argc == 1 || argc > 3) {
    printf("Usage: flan file.fl [output.ir]\n");
    exit(1);
  }

  log_debug_level = 0;
  identifiers = malloc(sizeof(array));
  array_new(identifiers);

  ts_init();

  // debug single file
  ast_t* root = fl_parse_main_file(argv[1]);
  // ast_t* root = fl_parse_file(argv[1]);
  if (ast_print_error(root)) {
    exit(1);
  }

  root = typesystem(root);
  if (ast_print_error(root)) {
    exit(1);
  }

  LLVMModuleRef module = fl_codegen(root, "test");

  if (argc == 3) {
    printf("export to ir %s\n", argv[2]);
    fl_to_ir(module, argv[2]);
  } else {
    printf("Running\n");
    // JIT has not been linked in.
    // cg_jit(module);
    fl_interpreter(module);
  }

  ts_exit();
  array_delete(identifiers);
  free(identifiers);
  ast_delete(root);

  return 0;
}
