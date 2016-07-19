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
#include "flang/libts.h"
#include "flang/libparser.h"
#include "flang/debug.h"

// return error
ast_t* ast_implement_fn(ast_t* call, ast_t* decl, string* uid) {
  assert(call->type == AST_EXPR_CALL);
  assert(decl->type == AST_DECL_FUNCTION);
  assert(decl->func.templated);

  ast_t* fn = ast_clone(decl);

  fn->func.from_tpl = fn; // comes from this template
  fn->func.templated = false;

  fn->func.uid = uid; // if 0 -> auto
  ast_parent(fn);

  ast_mk_insert_before(decl->parent, decl, fn);

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

// return error
ast_t* ast_implement_struct(ast_t* call, ast_t* decl, string* uid) {
  assert(call->type == AST_EXPR_CALL); // hackish :) for fun and profit!
  assert(decl->type == AST_DECL_STRUCT);
  assert(decl->structure.tpls != 0);

  ast_t* clone = ast_clone(decl);

  clone->structure.from_tpl = decl; // comes from this template
  clone->structure.tpls = 0;

  clone->structure.id = ast_mk_lit_id(uid, false);
  ast_parent(clone);

  ast_mk_insert_before(decl->parent, decl, clone);

  // todo replace types!
  u64 old;
  u64 new;

  ast_t* params = decl->structure.tpls;
  u64 count = params->list.count;

  u64 i;
  ty_t* param_ty;
  u64 param_ty_id;

  for (i = 0; i < count; ++i) {
    param_ty_id = params->list.elements[i]->ty_id;
    param_ty = &ts_type_table[param_ty_id];
    log_silly("param type %lu", param_ty_id);
    if (param_ty->of == FL_TEMPLATE) {
      // search type and replace!
      log_silly("replace type %lu to %lu", param_ty_id,
                call->call.arguments->list.elements[i]->ty_id);
      ast_replace_types(clone, param_ty_id,
                        call->call.arguments->list.elements[i]->ty_id);
    }
  }

  // ast_replace_types(fn, 21, 4);
  clone->ty_id = ty_create_struct(clone);
  typesystem(clone);

  call->call.decl = clone; // TODO this maybe be very dangerous!
  return clone;
}
