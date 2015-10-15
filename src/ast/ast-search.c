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

  ast_reverse(node, ast_search_id_decl_cb, 0, 0, (void*)identifier,
              (void*)&ret);

  return ret;
}

ast_action_t ast_search_fn_wargs_cb(ast_t* node, ast_t* parent, size_t level,
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

  ast_reverse(node, ast_search_fn_wargs_cb, 0, 0, (void*)input, (void*)&ret);

  return ret;
}
