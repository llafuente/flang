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

ast_action_t load_imports(ast_t* node, ast_t* parent, size_t level,
                          void* userdata_in, void* userdata_out) {
  if (node->type == FL_AST_IMPORT && !node->import.imported) {
    assert(node->parent->type == FL_AST_LIST);

    char filepath[1024] = "";
    strcat(filepath, "./../");
    strcat(filepath, node->import.path->string.value->value);
    strcat(filepath, ".fl");

    printf("load module %s\n", filepath);

    ast_t* module = fl_parse_file(filepath);
    if (ast_print_error(module)) {
      fl_fatal_error("Failed to load module: %s\n", filepath);
    }

    ast_mk_insert_before(node->parent, node, module);

    node->import.imported = true;
    ast_parent(node->parent); // update parents
    ((*(size_t*)userdata_out))++;
  }

  return FL_AC_CONTINUE;
}

// return error
ast_t* psr_load_imports(ast_t* node) {
  ast_dump(node);
  ast_parent(node);
  size_t imported;
  do {
    imported = 0;
    ast_traverse(node, load_imports, 0, 0, 0, (void*)&imported);
  } while (imported);

  return 0;
}
