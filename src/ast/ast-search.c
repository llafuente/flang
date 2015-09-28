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

bool ast_search_id_decl_cb(ast_t* node, ast_t* parent, size_t level,
                           void* userdata_in, void* userdata_out) {
  // printf("** ast_search_id_decl_cb **\n");
  // ast_dump(node);

  switch (node->type) {
  case FL_AST_PARAMETER: {
    string* id = (string*)userdata_in;

    if (st_cmp(id, node->func.id->identifier.string) == 0) {
      void** ret = (void**)userdata_out;
      *ret = node;
      return false;
    }
  }
  case FL_AST_DTOR_VAR: {
    string* id = (string*)userdata_in;

    if (st_cmp(id, node->var.id->identifier.string) == 0) {
      void** ret = (void**)userdata_out;
      *ret = node;
      return false;
    }
  }
  }

  return true;
}

ast_t* ast_search_id_decl(ast_t* node, string* identifier) {
  ast_t* ret = 0;

  ast_reverse(node, ast_search_id_decl_cb, 0, 0, (void*)identifier,
              (void*)&ret);

  return ret;
}

/*
ast_t* ast_search_id_decl(ast_t* node, string* name) {
  while ((node = node->parent) != 0) {
    switch (node->type) {
    case FL_AST_DECL_FUNCTION: {

      if (node->func.nparams) {
        size_t i = 0;
        ast_t* tmp;
        ast_t* list = node->func.params;
        while ((tmp = list->list.elements[i++]) != 0) {
          if (st_cmp(name, tmp->param.id->identifier.string) == 0) {
            log_verbose("found parameter @ [%zu]", i);
            return tmp;
          }
        }
      }
    } break;
    case FL_AST_BLOCK: {
      // search in the list
      size_t i = 0;
      ast_t* tmp;

      if (node->block.body) {
        while ((tmp = node->block.body[i++]) != 0) {
          if (tmp->type == FL_AST_DTOR_VAR &&
              st_cmp(name, tmp->var.id->identifier.string) == 0) {
            log_verbose("found var decl @ [%zu]", i);
            return tmp;
          }
        }
      }
    }
    default: {} // remove warn
    }
  }

  return 0;
}
*/
