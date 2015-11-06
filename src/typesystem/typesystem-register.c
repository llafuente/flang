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

size_t get_type(ast_t* node) {
  assert(node->type == FL_AST_TYPE);

  // empty var_decl for example
  if (!node->ty.id) {
    return 0;
  }

  // built-in
  if (strcmp(node->ty.id->identifier.string->value, "bool") == 0) {
    return node->ty_id = TS_BOOL;
  }
  if (strcmp(node->ty.id->identifier.string->value, "void") == 0) {
    return node->ty_id = TS_VOID;
  }
  if (strcmp(node->ty.id->identifier.string->value, "i8") == 0) {
    return node->ty_id = TS_I8;
  }
  if (strcmp(node->ty.id->identifier.string->value, "u8") == 0) {
    return node->ty_id = TS_U8;
  }
  if (strcmp(node->ty.id->identifier.string->value, "i16") == 0) {
    return node->ty_id = TS_I16;
  }
  if (strcmp(node->ty.id->identifier.string->value, "u16") == 0) {
    return node->ty_id = TS_U16;
  }
  if (strcmp(node->ty.id->identifier.string->value, "i32") == 0) {
    return node->ty_id = TS_I32;
  }
  if (strcmp(node->ty.id->identifier.string->value, "u32") == 0) {
    return node->ty_id = TS_U32;
  }
  if (strcmp(node->ty.id->identifier.string->value, "i64") == 0) {
    return node->ty_id = TS_I64;
  }
  if (strcmp(node->ty.id->identifier.string->value, "u64") == 0) {
    return node->ty_id = TS_U64;
  }
  if (strcmp(node->ty.id->identifier.string->value, "f32") == 0) {
    return node->ty_id = TS_F32;
  }
  if (strcmp(node->ty.id->identifier.string->value, "f64") == 0) {
    return node->ty_id = TS_F64;
  }
  if (strcmp(node->ty.id->identifier.string->value, "string") == 0) {
    // return node->ty_id = TS_CSTR; // TODO TS_STRING
    return node->ty_id = TS_STRING;
  }

  if (strcmp(node->ty.id->identifier.string->value, "ptr") == 0) {
    assert(node->ty.child != 0);
    size_t t = get_type(node->ty.child);
    return node->ty_id = ts_wapper_typeid(FL_POINTER, t);
  }

  if (strcmp(node->ty.id->identifier.string->value, "vector") == 0) {
    assert(node->ty.child != 0);
    printf("TYPE = %u\n", node->ty.child->type);
    size_t t = get_type(node->ty.child);
    return node->ty_id = ts_wapper_typeid(FL_VECTOR, t);
  }

  printf("delayed type!?");
  return 0;
}

ast_action_t register_types(ast_t* node, ast_t* parent, size_t level,
                            void* userdata_in, void* userdata_out) {
  if (node->ty_id)
    return FL_AC_CONTINUE;

  switch (node->type) {
  case FL_AST_DECL_STRUCT:
    ts_register_types(node->structure.fields);
    node->ty_id = ts_struct_create(node);
    break;
  case FL_AST_DECL_FUNCTION:
    // declare the function
    ts_register_types(node->func.params);
    ts_register_types(node->func.ret_type);
    node->ty_id = ts_fn_create(node);
    break;
  case FL_AST_TYPE: {
    // check wrappers
    ast_t* p = node->parent;
    get_type(node);

    switch (p->type) {
    case FL_AST_DECL_FUNCTION:
    case FL_AST_TYPE:
      // just ignore
      break;
    case FL_AST_DTOR_VAR:
      p->ty_id = node->ty_id;
      p->var.id->ty_id = node->ty_id;
      break;

    case FL_AST_PARAMETER:
      p->ty_id = node->ty_id;
      p->param.id->ty_id = node->ty_id;
      break;
    case FL_AST_DECL_STRUCT_FIELD:
      p->ty_id = node->ty_id;
      p->field.id->ty_id = node->ty_id;
      break;
    default: {
      ast_dump(p);
      log_error("what is this?!");
    }
    }
  }
  default: {} // supress warning
  }

  return FL_AC_CONTINUE;
}

// return error
ast_t* ts_register_types(ast_t* node) {
  ast_traverse(node, register_types, 0, 0, 0, 0);
  return node;
}
