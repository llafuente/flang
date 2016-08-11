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
#include "flang/libast.h"
#include "flang/libts.h"
#include "flang/debug.h"

ast_t* ast_search_id_decl(ast_t* node, string* identifier) {
  ast_t* ret = 0;
  array* arr = 0;

  char* cstr = identifier->value;

  do {
    node = ast_get_scope(node);

    // found a variable!
    ret = (ast_t*)hash_get(node->block.variables, cstr);
    if (ret) {
      return ret;
    }

    // a function cannot collide with a struct
    arr = hash_get(node->block.functions, cstr);
    if (arr) {
      return (ast_t*)array_get(arr, 0);
    }

  } while (node->block.scope != AST_SCOPE_GLOBAL);
  return 0;
}

ast_t* ast_search_fn(ast_t* node, string* identifier, u64* args, u64 nargs,
                     u64 ret_ty, bool var_args) {
  array* arr = ast_scope_fns(node, identifier);

  if (!arr) {
    return 0;
  }

  u64 i;
  ast_t* ret = 0;
  ast_t* fn;
  for (i = 0; i < arr->length; ++i) {
    fn = arr->values[i];
    if (st_cmp(identifier, fn->func.id->identifier.string) == 0) {
      log_verbose("function name found");
      ty_t t = ty(fn->ty_id);
      fl_assert(t.of == TY_FUNCTION);

      log_verbose("varargs %d == %d", t.func.varargs, var_args);
      log_verbose("nparams %zu == %zu", t.func.nparams, nargs);
      log_verbose("params %d",
                  memcmp(args, t.func.params, nargs * sizeof(u64)));
      log_verbose("ret %zu == %zu", t.func.ret, ret_ty);

      if (t.func.nparams == nargs &&
          memcmp(args, t.func.params, nargs * sizeof(u64)) == 0 &&
          t.func.varargs == var_args && t.func.ret == ret_ty) {
        ret = fn;
        goto cleanup;
      }
    }
  }

cleanup:
  array_delete(arr);
  pool_free(arr);
  return ret;
}

ast_t* ast_search_fn_op(ast_t* node, int operator, u64 ty_id) {
  char buffer[64];
  snprintf(buffer, 64, "operator_%d", operator);
  string* id = st_newc(buffer, st_enc_utf8);
  array* arr = ast_scope_fns(node, id);

  if (!arr) {
    return 0;
  }

  u64 i;
  ast_t* ret = 0;
  ast_t* fn;
  for (i = 0; i < arr->length; ++i) {
    fn = arr->values[i];
    ty_t t = ty(fn->ty_id);
    // binary atm!
    if (fn->func.operator== operator&& t.func.nparams == 2 &&
        t.func.params[0] == ty_id) {
      ret = fn;
      goto cleanup;
    }
  }

cleanup:
  array_delete(arr);
  pool_free(arr);
  return ret;
}

// TODO handle args
ast_t* ast_search_fn_wargs(string* id, ast_t* args_call) {
  // search function
  array* arr = ast_scope_fns(args_call->parent, id);
  bool checked = false;
  ast_t* ret_decl = 0;

  if (!arr) {
    // search a variable with function type
    log_verbose("undefined function: '%s' must be a variable", id->value);
    ast_t* var_decl = ast_scope_var(args_call, id);
    if (var_decl == 0) {
      // implicit declaration of function 'z' is invalid in C99
      ast_raise_error(
          args_call->parent,
          "type error, cannot find function or variable with given name: '%s'",
          id->value);
    }

    ty_t fn_ty = ty(var_decl->ty_id);
    if (fn_ty.of != TY_FUNCTION) {
      ast_raise_error(args_call->parent, "type error, called object type (%s) "
                                         "is not a function or function "
                                         "pointer",
                      ty_to_string(var_decl->ty_id)->value);
    }

    ret_decl = fn_ty.func.decl;
    goto cleanup;
  }

  log_verbose("declarations with same name = %lu\n", arr->length);

  if (arr->length == 1) {
    ret_decl = array_get(arr, 0);
    goto cleanup;
  }

  ast_t* decl;
  ast_t* params;
  ast_t* param;

  ast_t* arg_call;

  u64 i, j;
  u64 imax = args_call->list.length;
  u64 jmax = arr->length;

  // strict and no template
  for (j = 0; j < jmax; ++j) {
    decl = array_get(arr, j);
    if (ty_compatible_fn(decl->ty_id, args_call, true, false)) {
      checked = true;
      ret_decl = decl;
      goto cleanup;
    }
  }
  // castable and no template
  for (j = 0; j < jmax; ++j) {
    decl = array_get(arr, j);
    if (ty_compatible_fn(decl->ty_id, args_call, false, false)) {
      checked = true;
      ret_decl = decl;
      goto cleanup;
    }
  }
  // castable & template
  for (j = 0; j < jmax; ++j) {
    decl = array_get(arr, j);
    if (ty_compatible_fn(decl->ty_id, args_call, false, true)) {
      checked = true;
      ret_decl = decl;
      goto cleanup;
    }
  }

cleanup:

  if (!checked && ret_decl) {
    // check compatibility!
    if (!ty_compatible_fn(ret_decl->ty_id, args_call, false, true)) {
      ast_raise_error(
          args_call->parent,
          "type error, invalid arguments. Expected: (%s)\nFound: (%s)",
          ty_to_string(ret_decl->ty_id)->value,
          ty_to_string_list(args_call)->value);
    }
  }

  if (arr) {
    array_delete(arr);
    pool_free(arr);
  }

  return ret_decl;
}

ast_action_t __trav_get_list_node(ast_trav_mode_t mode, ast_t* node,
                                  ast_t* parent, u64 level, void* userdata_in,
                                  void* userdata_out) {
  if (mode == AST_TRAV_LEAVE)
    return 0;

  if (node->type == *(ast_types_t*)userdata_in) {
    array_push((array*)userdata_out, node);
  }

  return AST_SEARCH_CONTINUE;
}

array* ast_search_node_type(ast_t* node, ast_types_t t) {
  array* userdata = malloc(sizeof(array));
  array_new(userdata);

  ast_traverse(node, __trav_get_list_node, 0, 0, (void*)&t, (void*)userdata);

  if (userdata->length) {
    return userdata;
  }

  array_delete(userdata);
  free(userdata);
  return 0;
}
