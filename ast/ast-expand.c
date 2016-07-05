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
#include "flang/libast.h"
#include "flang/typesystem.h"
#include "flang/libparser.h"

// return error
ast_t* ast_expand_fn(ast_t* call, ast_t* decl) {
  assert(call->type == AST_EXPR_CALL);
  assert(decl->type == AST_DECL_FUNCTION);
  assert(decl->func.templated);

  ast_t* fn = ast_clone(decl);
  fn->func.uid = 0; // remove uid, so no collision, a new one
  ast_parent(fn);
  fn->parent = decl->parent;
  ast_mk_insert_before(decl->parent, decl, fn);

  fn->func.templated = false;

  // todo replace types!
  u64 old;
  u64 new;

  ast_t* params = fn->func.params;
  u64 count = params->list.count;

  u64 i;
  ty_t* param_ty;
  u64 param_ty_id;

  for (i = 0; i < count; ++i) {
    param_ty_id = params->list.elements[i]->ty_id;
    param_ty = &ts_type_table[param_ty_id];
    if (param_ty->of == FL_TEMPLATE) {
      // search type and replace!
      ast_replace_types(fn, param_ty_id,
                        call->call.arguments->list.elements[i]->ty_id);
    }
  }

  // ast_replace_types(fn, 21, 4);
  fn->ty_id = ty_create_fn(fn);
  typesystem(fn);

  call->call.decl = fn;
  return fn;
}
