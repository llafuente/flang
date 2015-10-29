#include <stdio.h>
#include "flang.h"

extern int yyparse(ast_t* root);

int main(int argc, char** argv) {
  ast_t* root;
  yyparse(root);

  ast_dump(root);

  //ast_delete(root);
  return 0;
}
