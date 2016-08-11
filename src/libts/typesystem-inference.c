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
#include "flang/libast.h"
#include "flang/debug.h"

struct __id_search {
  string* needle;
  ast_t** list;
  u64 length;
};

typedef struct __id_search __id_search_t;

ast_action_t __ast_find_identifier(ast_trav_mode_t mode, ast_t* node,
                                   ast_t* parent, u64 level, void* userdata_in,
                                   void* userdata_out) {
  if (mode == AST_TRAV_LEAVE)
    return 0;

  if (node->type == AST_LIT_IDENTIFIER) {
    __id_search_t* data = (__id_search_t*)userdata_in;
    string* str = data->needle;
    if (st_cmp(str, node->identifier.string) == 0) {
      data->list[data->length++] = node;
    }
  }
  return AST_SEARCH_CONTINUE;
}

ast_action_t __ts_inference_dtors(ast_trav_mode_t mode, ast_t* node,
                                  ast_t* parent, u64 level, void* userdata_in,
                                  void* userdata_out) {
  if (mode == AST_TRAV_LEAVE)
    return 0;

  if (node->type == AST_DTOR_VAR && node->var.type->ty_id == 0) {
    // search all ocurrences of this identifier
    __id_search_t data;
    data.list = calloc(sizeof(ast_t*), 100); // TODO resizable
    data.needle = node->var.id->identifier.string;
    data.length = 0;

    ast_traverse(node->parent, __ast_find_identifier, 0, 0, (void*)&data, 0);

    if (data.length) {
      // search from any aparience, if we can get the type
      u64 i = 0;
      for (; i < data.length; ++i) {
        ast_t* fnod = data.list[i];
        ast_t* parent = fnod->parent;

        // assignament at lhs
        if (parent->type == AST_EXPR_ASSIGNAMENT &&
            parent->assignament.left == fnod) {
          ast_t* rhs = parent->assignament.right;
          ts_pass(rhs);

          u64 t = rhs->ty_id;
          if (t) {
            // parent->ty_id = t; // assignament type
            // parent->assignament.left->ty_id = t; // lhs type
            // node->ty_id = t;
            node->var.type->ty_id = t;

            ((*(u64*)userdata_out))++;
            break;
          }
        }

        // as argument
        if (parent->type == AST_LIST) {
          ast_t* call = parent->parent;

          if (parent->parent->type == AST_EXPR_CALL) {
            string* callee = call->call.callee->identifier.string;
            ast_t* decl = ast_search_fn_wargs(callee, fnod);
            if (!decl) {
              continue;
            }
            u64 idx = 0;
            while (parent->list.values[idx] != fnod) {
              ++idx;
            }

            node->var.type->ty_id = decl->func.params->list.values[idx]->ty_id;

            ((*(u64*)userdata_out))++;
            break;
          }
        }
      }
    }

    free(data.list);
  }

  return AST_SEARCH_CONTINUE;
}

ast_action_t __ts_inference_fn_ret(ast_trav_mode_t mode, ast_t* node,
                                   ast_t* parent, u64 level, void* userdata_in,
                                   void* userdata_out) {
  if (mode == AST_TRAV_LEAVE)
    return 0;

  if (node->type == AST_DECL_FUNCTION && node->func.ret_type->ty_id == 0) {
    log_silly("inference: function return value");

    ast_t* ret = node->func.ret_type;
    ast_t* body = node->func.body;
    array* list = ast_search_node_type(node, AST_STMT_RETURN);

    if (list) {
      u64 i = 0;
      ast_t* el;
      u64 ct = 0;

      for (; i < list->length; ++i) {
        el = ((ast_t*)list->values[i])->ret.argument;
        if (!el->ty_id) {
          // this need inference...
          // wait for later...
          array_delete(list);
          free(list);
          return AST_SEARCH_CONTINUE;
        } else if (ct && ct != el->ty_id) {
          // TODO can we be more specific
          // double type? error!
          ast_raise_error(node, "Multiple return types found.");
        } else {
          ct = el->ty_id;
        }
      }
      node->func.ret_type->ty.id->ty_id = node->func.ret_type->ty_id = ct;

      array_delete(list);
      free(list);
    } else {
      // add return at last statement, and type to void
      node->func.ret_type->ty_id = TS_VOID;
      if (node->func.ret_type->ty.id) {
        node->func.ret_type->ty.id->ty_id = TS_VOID;
      }
      ast_t* ret = ast_mk_return(0);
      ret->ty_id = TS_VOID;
      ast_mk_list_push(body->block.body, ret);
      ast_parent(node);
    }

    // now update the type

    ts_type_table[node->ty_id].func.ret = node->func.ret_type->ty_id;
    ((*(u64*)userdata_out))++;
  }

  return AST_SEARCH_CONTINUE;
}

// return if modified something
bool ts_inference(ast_t* node) {
  // var x = 10; <- double
  // var x = ""; <- string*
  // var x = []; <- look usage
  // var x = [10]; <- array<double>

  // var i64 x = 10; <- cast 10 to i64
  u64 modified;
  bool ret = false;
  do {
    modified = 0;
    ast_traverse(node, __ts_inference_dtors, 0, 0, 0, (void*)&modified);
    ast_traverse(node, __ts_inference_fn_ret, 0, 0, 0, (void*)&modified);
    if (modified) {
      ret = true;
    }
  } while (modified);

  return ret;
}
