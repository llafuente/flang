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
#include "flang/libparser.h"
#include "flang/debug.h"

// return error

void __ast_implement_type_check(ast_t* node, u64 from, u64 to) {
  ty_t from_type = ty(from);
  ty_t to_type = ty(to);

  log_silly("to_type.of[%u] from_type.of[%u]", to_type.of, from_type.of);
  if (from_type.of == TY_REFERENCE && to_type.of != TY_REFERENCE) {
    ast_raise_error(node, "type error, cannot implement type (%s) into (%s). A "
                          "reference is required.",
                    ty_to_string(from)->value, ty_to_string(to)->value);
  }

  if (from_type.of == TY_STRUCT && to_type.of != TY_STRUCT) {
    ast_raise_error(node, "type error, cannot implement type (%s) into (%s). A "
                          "struct is required.",
                    ty_to_string(from)->value, ty_to_string(to)->value);
  }

  // TODO REVIEW here we can check the from_type is the father of to_type

  if (from_type.of == TY_POINTER && to_type.of != TY_POINTER) {
    ast_raise_error(node, "type error, cannot implement type (%s) into (%s). A "
                          "pointer is required.",
                    ty_to_string(from)->value, ty_to_string(to)->value);
  }

  // TODO REVIEW what about functions ?!
}

// from: decl
// to: implementation
void ast_implement_type_in_order(ast_t* fn, u64 from, u64 to) {
  fl_assert(fn->type == AST_DECL_FUNCTION);

  ty_t from_type = ty(from);
  ty_t to_type = ty(to);

  log_silly("from %lu[%s] to %lu[%s]", from, ty_to_string(from)->value, to,
            ty_to_string(to)->value);
  // check compatible/enforced types first.
  __ast_implement_type_check(fn, from, to);

  switch (from_type.of) {
  case TY_STRUCT: {
    u64 jmax = from_type.structure.members.length;
    for (u64 j = 0; j < jmax; ++j) {
      if (ty_is_template(from_type.structure.fields[j])) {
        log_silly("replace template idx %lu\n", j);
        ast_replace_types(fn, from_type.structure.fields[j],
                          to_type.structure.fields[j]);
      } else if (ty_is_templated(from_type.structure.fields[j])) {
        log_silly("implement templated idx %lu\n", j);
        ast_implement_type_in_order(fn, from_type.structure.fields[j],
                                    to_type.structure.fields[j]);
      }
    }
  } break;
  case TY_TEMPLATE:
    ast_replace_types(fn, from, to);
    break;
  case TY_REFERENCE:
    ast_replace_types(fn, from_type.ref.to, to_type.ref.to);
    ast_implement_type_in_order(fn, from_type.ref.to, to_type.ref.to);
    break;
  case TY_POINTER:
    ast_replace_types(fn, from_type.ptr.to, to_type.ptr.to);
    ast_implement_type_in_order(fn, from_type.ptr.to, to_type.ptr.to);
    break;
  default:
    ast_raise_error(fn, "TODO, not handled case atm 2 %s",
                    ty_to_string(from)->value);
  }
}
ast_t* ast_implement_fn(ast_t* type_list, ast_t* decl, string* uid) {
  fl_assert(type_list->type == AST_LIST);
  fl_assert(decl->type == AST_DECL_FUNCTION);
  fl_assert(decl->func.templated);

  ast_t* fn = ast_clone(decl);

  fn->func.from_tpl = fn; // comes from this template
  fn->func.templated = false;

  fn->func.uid = uid; // if 0 -> auto
  ast_parent(fn);

  log_silly("implement function %s", fn->func.id->identifier.string->value);
  ast_mk_insert_before(decl->parent, decl, fn);

  // todo replace types!
  u64 old;
  u64 new;

  ast_t* decl_params = fn->func.params;
  u64 count = decl_params->list.length;

  u64 i;
  u64 decl_param_ty_id;

  // loop left to right implementing each type and template
  for (i = 0; i < count; ++i) {
    decl_param_ty_id = decl_params->list.values[i]->ty_id;

    log_silly("type %lu - %s", i, ty_to_string(decl_param_ty_id)->value);

    if (ty_is_templated(decl_param_ty_id)) {
      ast_implement_type_in_order(fn, decl_param_ty_id,
                                  type_list->list.values[i]->ty_id);
      // search type and replace!
      ast_replace_types(fn, decl_param_ty_id, type_list->list.values[i]->ty_id);
    }
  }

  fn->ty_id = ty_create_fn(fn);
  _typesystem(fn);
  return fn;
}

// return error
ast_t* ast_implement_struct(ast_t* type_list, ast_t* decl, string* uid) {
  fl_assert(type_list->type == AST_LIST);
  fl_assert(decl->type == AST_DECL_STRUCT);
  fl_assert(decl->structure.tpls != 0);

  ast_t* clone = ast_clone(decl);

  clone->structure.from_tpl = decl; // comes from this template
  clone->structure.tpls = 0;

  if (uid == 0) {
    log_silly("create uid");
    ty_t tpl_type = ty(decl->ty_id);
    uid = st_clone(tpl_type.id);
    for (int i = 0; i < type_list->list.length; ++i) {
      st_append_char(&uid, '_');
      st_append(&uid, type_list->list.values[i]->ty.id->identifier.string);
    }
    log_silly("uid %s", uid->value);
  }

  clone->structure.id = ast_mk_lit_id(uid, false);
  ast_parent(clone);

  ast_mk_insert_before(decl->parent, decl, clone);

  // todo replace types!
  u64 old;
  u64 new;

  ast_t* params = decl->structure.tpls;
  u64 count = params->list.length;

  u64 i;
  ty_t* param_ty;
  u64 param_ty_id;

  for (i = 0; i < count; ++i) {
    param_ty_id = params->list.values[i]->ty_id;
    log_silly("param type %lu", param_ty_id);
    if (ty_is_template(param_ty_id)) {
      if (ty_is_templated(type_list->list.values[i]->ty_id)) {
        ast_raise_error(
            type_list->list.values[i],
            "type error, try to implement a template using another template");
      }
      // search type and replace!
      log_silly("replace type %lu to %lu", param_ty_id,
                type_list->list.values[i]->ty_id);
      ast_replace_types(clone, param_ty_id, type_list->list.values[i]->ty_id);
    }
  }

  clone->structure.templated = false;
  clone->ty_id = ty_create_struct(clone);
  clone->structure.id->ty_id = clone->ty_id;
  _typesystem(clone);

  ts_type_table[clone->ty_id].structure.from_tpl = decl->ty_id;

  return clone;
}
