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

#include "flang/common.h"
#include "flang/ast.h"
#include "flang/libast.h"
#include "flang/typesystem.h"
#include "flang/debug.h"

ast_action_t __trav_search_id_decl(ast_t* node, ast_t* parent, u64 level,
                                   void* userdata_in, void* userdata_out) {
#define COMPARE(x)                                                             \
  {                                                                            \
    string* id = (string*)userdata_in;                                         \
    if (st_cmp(id, x) == 0) {                                                  \
      void** ret = (void**)userdata_out;                                       \
      *ret = node;                                                             \
      return FL_AC_STOP;                                                       \
    }                                                                          \
  }

  switch (node->type) {
  case FL_AST_DECL_FUNCTION: {
    COMPARE(node->func.id->identifier.string);
    // skip function body / parameters
    return FL_AC_SKIP;
  }
  case FL_AST_PARAMETER: {
    COMPARE(node->param.id->identifier.string);
    break;
  }
  case FL_AST_DTOR_VAR: {
    COMPARE(node->var.id->identifier.string);
    break;
  }
  default: {} // supress warning
  }

  return FL_AC_CONTINUE;
}

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

ast_t* ast_search_fn(ast_t* node, string* identifier, u64* args,
                     u64 nargs, u64 ret_ty, bool var_args) {
  array* arr = ast_search_fns(node, identifier);

  if (!arr) {
    return 0;
  }

  u64 i;
  ast_t* fn;
  for (i = 0; i < arr->size; ++i) {
    fn = arr->data[i];
    if (st_cmp(identifier, fn->func.id->identifier.string) == 0) {
      log_verbose("function name found");
      ty_t t = ts_type_table[fn->ty_id];
      assert(t.of == FL_FUNCTION);

      log_verbose("varargs %d == %d", t.func.varargs, var_args);
      log_verbose("nparams %zu == %zu", t.func.nparams, nargs);
      log_verbose("params %d",
                  memcmp(args, t.func.params, nargs * sizeof(u64)));
      log_verbose("ret %zu == %zu", t.func.ret, ret_ty);

      if (t.func.nparams == nargs &&
          memcmp(args, t.func.params, nargs * sizeof(u64)) == 0 &&
          t.func.varargs == var_args && t.func.ret == ret_ty) {
        array_delete(arr);
        free(arr);
        return fn;
      }
    }
  }
  array_delete(arr);
  free(arr);
  return 0;
}

// TODO handle args
ast_t* ast_search_fn_wargs(string* id, ast_t* args_call) {
  // search function
  array* arr = ast_search_fns(args_call->parent, id);
  if (!arr) {
    // search a variable with function type
    log_verbose("undefined function: '%s' must be a variable", id->value);
    ast_t* decl = ast_search_id_decl(args_call, id);
    if (!decl) {
      ast_raise_error(args_call->parent,
                      "typesystem - cannot find function or variable: '%s'",
                      id->value);
    }
    // now search any function that has that ty_id
    // type is pointer to function so
    ty_t fn_ptr_ty = ts_type_table[decl->ty_id];
    if (fn_ptr_ty.of != FL_POINTER) {
      ast_raise_error(args_call->parent,
                      "typesystem - invalid variable type, not a function");
    }

    ty_t fn_ty = ts_type_table[fn_ptr_ty.ptr.to];
    if (fn_ty.of != FL_FUNCTION) {
      ast_raise_error(args_call->parent,
                      "typesystem - invalid variable type, not a function");
    }

    return fn_ty.func.decl;
  }

  log_verbose("declarations with same name = %d\n", arr->size);

  ast_t* ret_decl = 0;

  if (arr->size == 1) {
    ret_decl = array_get(arr, 0);
    goto fn_wargs_return;
  }

  ast_t* decl;
  ast_t* params;
  ast_t* param;

  ast_t* arg_call;

  u64 i, j;
  u64 imax = args_call->list.count;
  u64 jmax = arr->size;

  // strict and no template
  for (j = 0; j < jmax; ++j) {
    decl = array_get(arr, j);
    if (ty_compatible_fn(decl->ty_id, args_call, true, false)) {
      ret_decl = decl;
      goto fn_wargs_return;
    }
  }
  // castable and no template
  for (j = 0; j < jmax; ++j) {
    decl = array_get(arr, j);
    if (ty_compatible_fn(decl->ty_id, args_call, false, false)) {
      ret_decl = decl;
      goto fn_wargs_return;
    }
  }
  // castable & template
  for (j = 0; j < jmax; ++j) {
    decl = array_get(arr, j);
    if (ty_compatible_fn(decl->ty_id, args_call, false, true)) {
      ret_decl = decl;
      goto fn_wargs_return;
    }
  }

fn_wargs_return:
  array_delete(arr);
  free(arr);

  return ret_decl;
}

ast_action_t __search_fns(ast_t* node, ast_t* parent, u64 level,
                          void* userdata_in, void* userdata_out) {

  // log_verbose("traverse %d", node->type);

  if (node->type == FL_AST_DECL_FUNCTION) {
    string* ast_search_id = (string*)userdata_in;
    log_verbose("'%s' == '%s'", ast_search_id->value,
                node->func.id->identifier.string->value);
    if (st_cmp(ast_search_id, node->func.id->identifier.string) == 0) {
      log_verbose("function found push  !");
      array_append((array*)userdata_out, node);
    }
    // TODO retrive functions inside functions ?
    // it's not out of your scope?!
  }

  return FL_AC_CONTINUE;
}

array* ast_search_fns(ast_t* node, string* id) {
  array* arr = malloc(sizeof(array));
  array* arr2;
  array_new(arr);

  char* cstr = id->value;
  ast_t* scope = node;
  ast_t* fn;
  do {
    scope = ast_get_scope(scope);

    fn = (ast_t*)hash_get(scope->block.types, cstr);
    if (fn && fn->type == FL_AST_DECL_FUNCTION) {
      array_concat(arr, (array*)hash_get(scope->block.functions, cstr));
    }
  } while (scope->block.scope != AST_SCOPE_GLOBAL);

  if (arr->size) {
    return arr;
  }

  array_delete(arr);
  free(arr);
  return 0;
}

ast_action_t __trav_get_list_node(ast_t* node, ast_t* parent, u64 level,
                                  void* userdata_in, void* userdata_out) {
  if (node->type == *(ast_types_t*)userdata_in) {
    array_append((array*)userdata_out, node);
  }

  return FL_AC_CONTINUE;
}

array* ast_search_node_type(ast_t* node, ast_types_t t) {
  array* userdata = malloc(sizeof(array));
  array_new(userdata);

  ast_traverse(node, __trav_get_list_node, 0, 0, (void*)&t, (void*)userdata);

  if (userdata->size) {
    return userdata;
  }

  array_delete(userdata);
  free(userdata);
  return 0;
}
