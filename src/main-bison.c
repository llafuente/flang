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

  ast_delete(root);
  return 0;
}
