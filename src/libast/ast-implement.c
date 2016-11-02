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

typedef struct {
  u64 impl;
  u64 decl;
} impl_replace_t;

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

// array of impl_replace_t
void __get_types_to_replace_in_order(ast_t* decl_param, u64 decl_ty_id,
                                     u64 impl_ty_id, array* arr) {
  ty_t from_type = ty(decl_ty_id);
  ty_t to_type = ty(impl_ty_id);

  __ast_implement_type_check(decl_param, decl_ty_id, impl_ty_id);

  impl_replace_t* repl = pool_new(sizeof(impl_replace_t));
  repl->impl = impl_ty_id;
  repl->decl = decl_ty_id;

  array_push(arr, (void*)repl);

  log_silly("push replace type: %lu[%s] => %lu[%s]", decl_ty_id,
            ty_to_string(decl_ty_id)->value, impl_ty_id,
            ty_to_string(impl_ty_id)->value);

  switch (from_type.of) {
  case TY_STRUCT: {
    u64 jmax = from_type.structure.members.length;
    for (u64 j = 0; j < jmax; ++j) {
      __get_types_to_replace_in_order(decl_param, from_type.structure.fields[j],
                                      to_type.structure.fields[j], arr);
    }
  } break;
  case TY_REFERENCE:
    __get_types_to_replace_in_order(decl_param, from_type.ref.to,
                                    to_type.ref.to, arr);
    break;
  case TY_POINTER:
    __get_types_to_replace_in_order(decl_param, from_type.ptr.to,
                                    to_type.ptr.to, arr);
    break;
  default: {}
  }
}

ast_t* ast_implement_fn(ast_t* type_list, ast_t* decl, string* uid) {
  fl_assert(type_list->type == AST_LIST);
  return ast_implement_fn2(ast_list_get_types(type_list), decl, uid);
}

ast_t* ast_implement_fn2(array* type_list, ast_t* decl, string* uid) {
  fl_assert(decl->type == AST_DECL_FUNCTION);
  fl_assert(decl->func.templated);

  ast_t* impl = ast_clone(decl);

  impl->func.from_tpl = decl; // comes from this template
  impl->func.templated = false;

  impl->func.uid = uid; // if 0 -> auto

  log_silly("implement function %s as %s",
            decl->func.id->identifier.string->value,
            uid ? uid->value : "auto-name");
  ast_mk_insert_before(decl->parent, decl, impl);

  // todo replace types!

  ast_t* decl_params = impl->func.params;
  u64 count = decl_params->list.length;

  if (type_list->length != count) {
    // TODO where!
    ast_raise_error(decl, "syntax error, try to implement a function with %lu "
                          "parameters, expected %lu",
                    type_list->length, count);
  }

  u64 i;
  u64 j;
  ast_t* decl_param;
  u64 decl_param_ty_id;
  u64 impl_ty_id;

  array* types_to_replace = pool_new(sizeof(array));
  for (i = 0; i < count; ++i) {
    decl_param = decl_params->list.values[i];
    decl_param_ty_id = decl_param->ty_id;
    impl_ty_id = (u64)type_list->values[i];

    __get_types_to_replace_in_order(decl_param, decl_param_ty_id, impl_ty_id,
                                    types_to_replace);
  }

  // check a type won't be replaced twice with different types
  log_silly("replace TYPE/IDS\n\n\n\n\n");
  for (i = 0; i < types_to_replace->length; ++i) {
    impl_replace_t* repl = types_to_replace->values[i];
    log_silly("replace index[%lu] [%lu] to [%lu]", i, repl->decl, repl->impl);
    for (j = i + 1; j < types_to_replace->length; ++j) {
      impl_replace_t* repl2 = types_to_replace->values[j];

      if (repl2->decl == repl->decl && repl2->impl != repl->impl) {
        ast_raise_error(decl, "type error, try to implement '%s' with two "
                              "different types '%s' and '%s'",
                        ty_to_string(repl2->decl)->value,
                        ty_to_string(repl2->impl)->value,
                        ty_to_string(repl->impl)->value);
      }
    }
    // this must prevail!
    if (repl->decl != repl->impl) {
      string* decl_id = ty(repl->decl).id;
      string* impl_id = ty(repl->impl).id;
      if (decl_id != 0 && impl_id != 0) {
        ast_replace_identifiers(impl, decl_id, impl_id);
      }
      ast_replace_types(impl, repl->decl, repl->impl);
    }
  }

  impl->ty_id = 0;
  ast_reset_types(impl->func.body);

  _typesystem(impl);
  //ast_dump(impl);
  //exit(1001);

  return impl;
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
