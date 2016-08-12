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
#include "flang/flang.h"
#include "flang/libts.h"
#include "flang/debug.h"
#include "flang/libast.h"

ast_action_t __trav_implement(ast_trav_mode_t mode, ast_t* node, ast_t* parent,
                              u64 level, void* userdata_in,
                              void* userdata_out) {
  if (mode == AST_TRAV_LEAVE)
    return AST_SEARCH_SKIP;

  switch (node->type) {
  case AST_IMPLEMENT: {
    if (node->ts_passes)
      return AST_SEARCH_SKIP;

    node->ts_passes = 1;

    string* fn_id = node->impl.type_id->identifier.string;
    array* arr = ast_scope_fns(node, fn_id);

    if (!arr) {
      // maybe a struct ?!
      ast_t* decl = ast_scope_type(node, fn_id);
      if (decl && decl->type == AST_DECL_STRUCT) {
        // STRUCT
        ast_t* tmp = ast_implement_struct(node->impl.type_list, decl,
                                          node->impl.uid->identifier.string);
        log_silly("fn expanded: %zu", tmp->ty_id);
        return AST_SEARCH_SKIP;
      }

      ast_raise_error(node,
                      "typesystem - Cannot find function or struct named: '%s'",
                      fn_id->value);
    } else {
      ast_t* fn = 0;
      ast_t* tmp = 0;
      for (int i = 0; i < arr->length; ++i) {
        tmp = (ast_t*)arr->values[i];
        if (tmp->func.templated) {
          if (fn) {
            // raise! double template!
            ast_dump_s(fn);
            ast_dump_s(tmp);
            ast_raise_error(
                node,
                "typesystem - Cannot implement multiple functions (atm?)");
          }
          fn = tmp;
        }
      }

      if (!fn) {
        ast_raise_error(node,
                        "typesystem - Cannot find function '%s' with templates",
                        fn_id->value);
      } else {
        ast_t* tmp = ast_implement_fn(node->impl.type_list, arr->values[0],
                                      node->impl.uid->identifier.string);
        log_silly("fn expanded: %zu", tmp->ty_id);
      }
    }
    array_delete(arr);
    pool_free(arr);
  } break;
  default: {}
  }

  return AST_SEARCH_CONTINUE;
}

// return error
ast_t* ts_implement(ast_t* node) {
  ast_traverse(node, __trav_implement, 0, 0, 0, 0);

  return 0;
}
