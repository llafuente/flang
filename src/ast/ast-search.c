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

ast_action_t __trav_search_id_decl(ast_t* node, ast_t* parent, size_t level,
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

  ast_reverse(node, __trav_search_id_decl, 0, 0, (void*)identifier,
              (void*)&ret);

  return ret;
}

ast_action_t __ast_search_fn(ast_t* node, ast_t* parent, size_t level,
                             void* userdata_in, void* userdata_out) {

  if (node->type == FL_AST_DECL_FUNCTION) {
    void** ui = (void**)userdata_in;
    string* id = (string*)ui[0];

    if (st_cmp(id, node->func.id->identifier.string) == 0) {
      log_verbose("function name found");
      ty_t t = ts_type_table[node->ty_id];
      assert(t.of == FL_FUNCTION);

      size_t* args = (size_t*)ui[1];
      size_t nargs = (size_t)ui[2];
      size_t ret_ty = (size_t)ui[3];
      bool var_args = (bool)ui[4];

      log_verbose("varargs %d == %d", t.func.varargs, var_args);
      log_verbose("nparams %zu == %zu", t.func.nparams, nargs);
      log_verbose("params %d",
                  memcmp(args, t.func.params, nargs * sizeof(size_t)));
      log_verbose("ret %zu == %zu", t.func.ret, ret_ty);

      if (t.func.nparams == nargs &&
          memcmp(args, t.func.params, nargs * sizeof(size_t)) == 0 &&
          t.func.varargs == var_args && t.func.ret == ret_ty) {
        void** ret = (void**)userdata_out;
        *ret = node;
        // check arguments
        return FL_AC_STOP;
      }
    }
    return FL_AC_SKIP;
  }

  return FL_AC_CONTINUE;
}

ast_t* ast_search_fn(ast_t* node, string* identifier, size_t* args,
                     size_t nargs, size_t ret_ty, bool var_args) {
  ast_t* ret = 0;

  void* input[5];
  input[0] = (void*)identifier;
  input[1] = (void*)args;
  input[2] = (void*)nargs;
  input[3] = (void*)ret_ty;
  input[4] = (void*)var_args;

  ast_reverse(node, __ast_search_fn, 0, 0, (void*)input, (void*)&ret);

  return ret;
}

// TODO handle args
ast_t* ast_search_fn_wargs(string* id, ast_t* args_call) {
  array* arr = ast_search_fns(args_call->parent, id);
  if (!arr) {
    log_verbose("undefined function: '%s' must be a variable", id->value);
    ast_t* decl = ast_search_id_decl(args_call->parent, id);
    if (!decl) {
      log_error("no function/var: '%s'", id->value);
    }
    // now search any function that has that ty_id
    // type is pointer to function so
    size_t fn_ty = ts_type_table[decl->ty_id].ptr.to;
    return ts_type_table[fn_ty].func.decl;
  }

  log_verbose("declarations with same name = %d\n", arr->size);

  if (arr->size == 1) {
    ast_t* ret = array_get(arr, 0);
    array_delete(arr);
    free(arr);
    return ret;
  }

  ast_t* decl;
  ast_t* params;
  ast_t* param;
  ast_t* ret_decl = 0;

  ast_t* arg_call;

  size_t i, j;
  size_t imax = args_call->list.count;
  size_t jmax = arr->size;

  for (j = 0; j < jmax; ++j) {
    decl = array_get(arr, j);
    assert(decl->type == FL_AST_DECL_FUNCTION);

    params = decl->func.params;
    // get types from arguments first

    for (i = 0; i < imax; ++i) {
      arg_call = args_call->list.elements[i];
      param = params->list.elements[i];

      if (!arg_call->ty_id) {
        log_error("cannot find type of argument %zu", i);
      }

      if (!ts_castable(param->ty_id, arg_call->ty_id)) {
        break;
      }
      if (i == imax - 1) {
        // we reach the end all is ok!
        // this is compatible!
        ret_decl = decl;
      }
    }
  }

  array_delete(arr);
  free(arr);

  return ret_decl;
}

ast_action_t __search_fns(ast_t* node, ast_t* parent, size_t level,
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
  array* userdata = malloc(sizeof(array));
  array_new(userdata);

  log_verbose("ast_search_fns %s %p", id->value, node);

  ast_reverse(node, __search_fns, 0, 0, (void*)id, (void*)userdata);

  if (userdata->size) {
    return userdata;
  }

  array_delete(userdata);
  free(userdata);
  return 0;
}

ast_action_t __trav_search_fn_decl(ast_t* node, ast_t* parent, size_t level,
                                   void* userdata_in, void* userdata_out) {

  if (node->type == FL_AST_DECL_FUNCTION) {
    string* id = (string*)userdata_in;

    if (st_cmp(id, node->func.id->identifier.string) == 0) {
      void** ret = (void**)userdata_out;
      *ret = node;
      return FL_AC_STOP;
    }
  }

  return FL_AC_CONTINUE;
}

ast_t* ast_search_fn_decl(ast_t* identifier) {
  assert(identifier->type == FL_AST_LIT_IDENTIFIER);

  ast_t** ret;

  ast_reverse(identifier, __trav_search_fn_decl, 0, 0,
              (void*)identifier->identifier.string, (void*)ret);

  return *ret;
}

ast_action_t __trav_get_list_node(ast_t* node, ast_t* parent, size_t level,
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
