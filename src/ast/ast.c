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

ast_action_t ast_parent_cb(ast_t* node, ast_t* parent, size_t level,
                           void* userdata_in, void* userdata_out) {
  node->parent = parent;

  return FL_AC_CONTINUE;
}

void ast_parent(ast_t* root) { ast_traverse(root, ast_parent_cb, 0, 0, 0, 0); }

size_t ast_get_typeid(ast_t* node) {
  assert(node != 0);
  // check AST is somewhat "type-related"
  switch (node->type) {
  case FL_AST_DTOR_VAR:
    log_verbose("var decl '%s' T(%zu)", node->var.id->identifier.string->value,
                node->var.type->ty_id);
    return node->var.type->ty_id;
  case FL_AST_PARAMETER:
    log_verbose("parameter T(%zu)", node->param.id->ty_id);
    return node->param.id->ty_id;
  case FL_AST_EXPR_CALL: {
    // search function
  }
  case FL_AST_EXPR_MEMBER:
  case FL_AST_EXPR_BINOP: {
    // this is valid only after ts_pass
    return node->ty_id;
  }
  case FL_AST_EXPR_ASSIGNAMENT: {
    return ast_get_typeid(node->assignament.left);
  }
  case FL_AST_LIT_IDENTIFIER: {
    if (node->ty_id) {
      return node->ty_id;
    }

    log_verbose("identifier T(%s)", node->identifier.string->value);
    // search var-dtor and return it
    ast_t* dtor = ast_search_id_decl(node, node->identifier.string);
    if (dtor) {
      return ast_get_typeid(dtor);
    }
  } break;
  case FL_AST_TYPE:
    log_verbose("type T(%zu)", node->ty_id);
    return node->ty_id;
  case FL_AST_CAST:
    return node->ty_id;
  case FL_AST_LIT_BOOLEAN:
    return 2; // TODO maybe 3, i8
  case FL_AST_LIT_FLOAT:
  case FL_AST_LIT_INTEGER:
    return node->ty_id;
  // dont give information, continue up
  case FL_AST_EXPR_LUNARY:
    return ast_get_typeid(node->parent);
  case FL_AST_LIT_STRING:
    return TS_STRING;
  case FL_AST_DECL_FUNCTION:
    return node->ty_id;
  default: {}
  }
  log_error("ast_get_typeid: node is not type related! %d", node->type);
  return 0;
}

bool ast_is_pointer(ast_t* node) {
  size_t id = ast_get_typeid(node);
  return ts_type_table[id].of == FL_POINTER;
}

size_t ast_ret_type(ast_t* node) {
  switch (node->type) {
  case FL_AST_EXPR_ASSIGNAMENT:
    return ast_ret_type(node->assignament.right);
  case FL_AST_LIT_INTEGER:
  case FL_AST_LIT_FLOAT:
    return node->ty_id;
  default: { log_error("ast_ret_type: cannot find type!"); }
  }

  return 0;
}

ast_action_t ast_find_fn_decl_cb(ast_t* node, ast_t* parent, size_t level,
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

ast_t* ast_find_fn_decl(ast_t* identifier) {
  if (identifier->type != FL_AST_LIT_IDENTIFIER) {
    log_error("(ast_find_fn_decl) must be an identifier!");
  }
  ast_t** ret;

  ast_reverse(identifier, ast_find_fn_decl_cb, 0, 0,
              (void*)identifier->identifier.string, (void*)ret);

  return *ret;
}

FL_EXTERN size_t ast_get_struct_prop_idx(ast_t* decl, string* id) {
  size_t i;
  ast_t* list = decl->structure.fields;
  ast_t** elements = list->list.elements;
  size_t length = list->list.count;

  for (i = 0; i < length; ++i) {
    if (st_cmp(elements[i]->field.id->identifier.string, id)) {
      return i;
    }
  }
  return -1;
}
