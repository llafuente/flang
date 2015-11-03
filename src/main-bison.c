#include <stdio.h>
#include "flang.h"

extern int yyparse(ast_t** root);
array* identifiers;

int main(int argc, char** argv) {
  // TODO 0 means REPL
  if (argc == 1 || argc > 3) {
    printf("Usage: flan file.fl [output.ir]\n");
    exit(1);
  }

  log_debug_level = 10;
  identifiers = malloc(sizeof(array));
  array_new(identifiers);

  // ast_t* root;
  // yyparse(&root);
  ast_t* root = fl_parse_file(argv[1], true);
  if (ast_print_error(root)) {
    exit(1);
  }

  array_delete(identifiers);
  free(identifiers);

  // do inference
  ast_parent(root); // set node->parent

  ast_dump(root);
  root = ts_pass(root);
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
