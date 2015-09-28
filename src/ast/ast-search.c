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

ast_action_t ast_search_id_decl_cb(ast_t* node, ast_t* parent, size_t level,
                                   void* userdata_in, void* userdata_out) {
  // printf("\n** ");
  // ast_dump_one(node);
  // printf("\n");

  switch (node->type) {
  case FL_AST_DECL_FUNCTION: {
    // printf("FUNCTION!**** must be skipped!!!");
    return FL_AC_SKIP;
  }
  case FL_AST_PARAMETER: {
    string* id = (string*)userdata_in;

    if (st_cmp(id, node->func.id->identifier.string) == 0) {
      void** ret = (void**)userdata_out;
      *ret = node;
      return FL_AC_STOP;
    }
  }
  case FL_AST_DTOR_VAR: {
    string* id = (string*)userdata_in;

    if (st_cmp(id, node->var.id->identifier.string) == 0) {
      void** ret = (void**)userdata_out;
      *ret = node;
      return FL_AC_STOP;
    }
  }
  }

  return FL_AC_CONTINUE;
}

ast_t* ast_search_id_decl(ast_t* node, string* identifier) {
  ast_t* ret = 0;

  // printf("\nsearching '%s'\n", identifier->value);

  ast_reverse(node, ast_search_id_decl_cb, 0, 0, (void*)identifier,
              (void*)&ret);

  return ret;
}
