/*
* Copyright 2015 Luis Lafuente <llafuente@noboxout.com>
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
* 1. Redistributions of source code must retain the above copyright
* notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in the
* documentation and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
* THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "flang.h"
#include <libgen.h>

ast_action_t __trav_load_imports(ast_t* node, ast_t* parent, size_t level,
                                 void* userdata_in, void* userdata_out) {
  if (node->type == FL_AST_IMPORT && !node->import.imported) {
    assert(node->parent->type == FL_AST_LIST);

    char* file = node->import.path->string.value->value;

    char filepath[1024] = "";

    if (file[0] == '.' && file[1] == '/') {
      char* file2 = strdup(file);
      ast_t* root = ast_get_root(node);

      ast_dump(root);
      printf("??? %s\n\n\n", root->program.file);

      strcat(filepath, dirname(root->program.file));
      strcat(filepath, "/");
      strcat(filepath, file + 2);
      strcat(filepath, ".fl");
      free(file2);
    } else {
      strcat(filepath, "./../");
      strcat(filepath, file);
      strcat(filepath, ".fl");
    }

    // printf("load module %s\n", filepath);

    ast_t* module = fl_parse_file(filepath);
    if (ast_print_error(module)) {
      fl_fatal_error("Failed to load module: %s\n", filepath);
    }

    module->type = FL_AST_MODULE;

    ast_mk_insert_before(node->parent, node, module);

    node->import.imported = true;
    ast_parent(module); // update parents
    node->parent = module->parent;

    ((*(size_t*)userdata_out))++;
  }

  return FL_AC_CONTINUE;
}

// return error
ast_t* ast_load_imports(ast_t* node) {
  size_t imported;
  do {
    imported = 0;
    ast_traverse(node, __trav_load_imports, 0, 0, 0, (void*)&imported);
  } while (imported);

  return 0;
}
