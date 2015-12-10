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

void __ast_reduce_log(ast_t* node) {
  // this will mutate to expr-call
  // get each part code and inject it before in the list
  ast_t* list = node->log.list; // cache

  size_t i;
  char buffer[256];
  buffer[0] = '\0';

  for (i = 0; i < list->list.count; ++i) {
    ast_t* el = list->list.elements[i];
    // LLVMValueRef cgel = cg_ast_loaded("log", el, FL_CODEGEN_PASSTHROUGH);

    ast_t* lit = ast_mk_lit_string2(ast_get_code(el), false, true);
    lit->parent = list;
    ast_mk_list_insert(list, lit, i);
    ++i; // advance another one, because we insert before current
    strcat(buffer, "%s \x1B[32m");
    strcat(buffer, ty_to_printf(el->ty_id));
    strcat(buffer, "\x1B[39m ");
  }
  buffer[strlen(buffer) - 1] = '\n';
  ast_mk_list_insert(list, ast_mk_lit_string(buffer, false), 0);

  node->type = FL_AST_EXPR_CALL;
  node->call.callee = ast_mk_lit_id(st_newc("printf", st_enc_utf8), true);
  node->call.arguments = list;
  node->call.narguments = list->list.count;
  // typesystem need to pass again...
  node->call.callee->parent = node;
}

ast_action_t __trav_reduced(ast_t* node, ast_t* parent, size_t level,
                            void* userdata_in, void* userdata_out) {
  switch (node->type) {
  case FL_AST_STMT_LOG: {
    __ast_reduce_log(node);
    ++(*(size_t*)userdata_out);
  } break;
  default: {} // avoid warning
  }

  return FL_AC_CONTINUE;
}

// return error
ast_t* ast_reduce(ast_t* node) {
  size_t reduced;
  do {
    reduced = 0;
    ast_traverse(node, __trav_reduced, 0, 0, 0, (void*)&reduced);
    if (reduced) {
      typesystem(node);
    }
  } while (reduced);

  return node;
}
