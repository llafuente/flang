#include <stdio.h>
#include "flang.h"

extern int yyparse(ast_t** root);
array* identifiers;

int main(int argc, char** argv) {
  log_debug_level = 10;
  ast_t* root;
  identifiers = malloc(sizeof(array));
  array_new(identifiers);
  yyparse(&root);
  array_delete(identifiers);
  free(identifiers);

  ast_dump(root);

  if (ast_print_error(root)) {
    exit(1);
  }

  LLVMModuleRef module = fl_codegen(root, "test");
  ts_exit();

  if (argc == 3) {
    printf("export to ir %s\n", argv[2]);
    fl_to_ir(module, argv[2]);
  } else {
    printf("Running\n");
    // JIT has not been linked in.
    // cg_jit(module);
    fl_interpreter(module);
  }

  ast_delete(root);

  return 0;
}
