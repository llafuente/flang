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

hashtable_t* ts_hashtable = 0;
fl_type_t* fl_type_table = 0;
size_t fl_type_size = 0;

void ts_init() {
  if (!ts_hashtable) {
    ts_hashtable = ht_create(65536);
  }

  if (!fl_type_table) {
    fl_type_table = calloc(sizeof(fl_type_t), 100);

    // 0 means infer!
    fl_type_table[0].of = FL_INFER;
    // [1] void
    fl_type_table[1].of = FL_VOID;
    fl_type_table[1].id = st_newc("void", st_enc_ascii);

    size_t id = 2;
    // [2] bool
    fl_type_table[id].of = FL_NUMBER;
    fl_type_table[id].id = st_newc("bool", st_enc_ascii);
    fl_type_table[id].number.bits = 1;
    fl_type_table[id].number.fp = false;
    fl_type_table[id].number.sign = false;
    // [3-10] i8,u8,i16,u16,i32,u32,i64,u64
    size_t i = 3;
    char buffer[20];
    for (; i < 7; i++) {
      size_t bits = pow(2, i);
      fl_type_table[++id].of = FL_NUMBER;
      fl_type_table[id].number.bits = bits;
      fl_type_table[id].number.fp = false;
      fl_type_table[id].number.sign = false;
      sprintf(buffer, "u%d", bits);
      fl_type_table[id].id = st_newc(buffer, st_enc_ascii);

      fl_type_table[++id].of = FL_NUMBER;
      fl_type_table[id].number.bits = bits;
      fl_type_table[id].number.fp = false;
      fl_type_table[id].number.sign = true;
      sprintf(buffer, "i%d", bits);
      fl_type_table[id].id = st_newc(buffer, st_enc_ascii);
    }

    // [11] f32
    fl_type_table[++id].of = FL_NUMBER;
    fl_type_table[id].id = st_newc("f32", st_enc_ascii);
    fl_type_table[id].number.bits = 32;
    fl_type_table[id].number.fp = true;
    fl_type_table[id].number.sign = true;

    // [12] f64
    fl_type_table[++id].of = FL_NUMBER;
    fl_type_table[id].id = st_newc("f64", st_enc_ascii);
    fl_type_table[id].number.bits = 64;
    fl_type_table[id].number.fp = true;
    fl_type_table[id].number.sign = true;

    // [13+] user defined atm
    fl_type_size = ++id;
  }
}

bool ts_is_number(size_t id) {
  fl_type_t t = fl_type_table[id];
  return t.of == FL_NUMBER;
}

bool ts_is_fp(size_t id) {
  fl_type_t t = fl_type_table[id];
  return t.of == FL_NUMBER ? t.number.fp : false;
}

bool ts_is_int(size_t id) {
  fl_type_t t = fl_type_table[id];
  return t.of == FL_NUMBER ? !t.number.fp : false;
}

size_t ts_get_bigger_typeid(size_t a, size_t b) {
  fl_type_t t_a = fl_type_table[a];
  fl_type_t t_b = fl_type_table[b];

  if (t_a.of == FL_NUMBER && t_b.of == FL_NUMBER) {
    // check floating point
    if (t_a.number.fp && !t_b.number.fp) {
      return a;
    }

    if (t_b.number.fp && !t_a.number.fp) {
      return b;
    }
    // check different sign
    if (t_b.number.sign != t_a.number.sign) {
      // TODO grow type ?
      return b;
    }

    // check bits
    return t_a.number.bits > t_b.number.bits ? a : b;
  }

  // only numbers?
  cg_print("(typesystem) ts_get_bigger_typeid of %d & %d\n", t_a.of, t_b.of);

  return a;
}

bool ts_pass_cb(fl_ast_t* node, fl_ast_t* parent, size_t level,
                void* userdata) {
#define CREATE_CAST(cast, node, type_id)                                       \
  fl_ast_t* cast = (fl_ast_t*)calloc(1, sizeof(fl_ast_t));                     \
  cast->token_start = 0;                                                       \
  cast->token_end = 0;                                                         \
  cast->type = FL_AST_CAST;                                                    \
  cast->parent = node->parent;                                                 \
  node->parent = cast;                                                         \
  cast->cast.element = node;                                                   \
  cast->ty_id = type_id;

  switch (node->type) {
  case FL_AST_EXPR_CALL: {
    fl_ast_t* fdecl = fl_ast_find_fn_decl(node->call.callee);
    // if (!fdecl) {
    //  cg_error("(ts) cannot find function %s\n",
    //  node->call.callee->identifier.string->value);
    //}
    cg_print("(typesystem) ret [%p]\n", fdecl);
    if (fdecl) {
      // fl_ast_debug(fdecl);

      node->ty_id = fl_ast_get_typeid(fdecl->func.ret_type);
    }

  } break;
  case FL_AST_EXPR_ASSIGNAMENT: {
    // fl_ast_debug(node);

    size_t l_type = fl_ast_get_typeid(node->assignament.left);

    fl_ast_t* r = node->assignament.right;
    size_t r_type = fl_ast_get_typeid(r);
    if (!r_type) {
      ts_pass(r);
      r_type = r->ty_id;
    }

    if (l_type != r_type) {
      dbg_debug("(typesystem) assignament cast [%zu - %zu]\n", l_type, r_type);
      // TODO check cast if valid!

      CREATE_CAST(cast, r, l_type);

      node->assignament.right = cast;
      node->ty_id = l_type;
    }
  } break;
  case FL_AST_EXPR_BINOP: {
    cg_print("(typesystem) binop found %d\n", node->binop.operator);
    fl_ast_debug(node);
    // cast if necessary
    fl_ast_t* l = node->binop.left;
    fl_ast_t* r = node->binop.right;

    // operation that need casting or fp/int
    size_t l_type = fl_ast_get_typeid(l);
    if (!l_type) {
      ts_pass(l);
      l_type = l->ty_id;
    }

    size_t r_type = fl_ast_get_typeid(r);
    if (!r_type) {
      ts_pass(r);
      r_type = r->ty_id;
    }

    bool l_fp = ts_is_fp(l_type);
    bool r_fp = ts_is_fp(r_type);

    // binop
    switch (node->binop.operator) {
    case FL_TK_AND:
    case FL_TK_OR:
    case FL_TK_CARET:
    case FL_TK_LT2:
    case FL_TK_GT2:
      // left and right must be Integers!
      if (l_fp || r_fp) {
        cg_error("invalid operants\n");
      }
      break;
    case FL_TK_EQUAL2:
    case FL_TK_EEQUAL: // !=
    case FL_TK_LTE:
    case FL_TK_LT:
    case FL_TK_GTE:
    case FL_TK_GT: {
      // TODO this should test if any side is a literal
      // TEST parser-expression-test.c:187

      // both sides must be the same! the bigger one
      node->ty_id = ts_get_bigger_typeid(l_type, r_type);
      if (node->ty_id != l_type) {
        // cast left side
        CREATE_CAST(cast, l, node->ty_id);
        node->binop.left = cast;
      }

      if (node->ty_id != r_type) {
        // cast right side
        CREATE_CAST(cast, r, node->ty_id);
        node->binop.right = cast;
      }

    } break;
    default: {
      // TODO check fp-vs-int -> cast
      node->ty_id = ts_get_bigger_typeid(l_type, r_type);

      if (!l_fp && !r_fp) {
        // TODO handle sign
      } else if (l_fp && r_fp) {
      } else if (l_fp && !r_fp) {
        // upcast right
        fl_ast_t* cast = (fl_ast_t*)calloc(1, sizeof(fl_ast_t));
        cast->token_start = 0;
        cast->token_end = 0;
        cast->type = FL_AST_CAST;
        cast->ty_id = l_type;
        cast->cast.element = node->binop.right;
        node->binop.right = cast;
        node->ty_id = l_type;
      } else {
        fl_ast_t* cast = (fl_ast_t*)calloc(1, sizeof(fl_ast_t));
        cast->token_start = 0;
        cast->token_end = 0;
        cast->type = FL_AST_CAST;
        cast->ty_id = r_type;
        cast->cast.element = node->binop.left;
        node->binop.left = cast;
        node->ty_id = r_type;
      }
    }
    }
  }
  }
  return true;
}

fl_ast_t* ts_pass(fl_ast_t* node) {
  cg_print("(ts) pass start!\n");

  fl_ast_traverse(node, ts_pass_cb, 0, 0, 0);
}

// wrapper types are
// * FL_POINTER
// * FL_VECTOR
size_t ts_wapper_typeid(fl_types_t wrapper, size_t child) {
  size_t i;

  for (i = 0; i < fl_type_size; ++i) {
    if (fl_type_table[i].of == wrapper && fl_type_table[i].ptr.to == child) {
      return i;
    }
  }
  // add it!
  i = fl_type_size++;
  switch (wrapper) {
  case FL_POINTER:
    fl_type_table[i].of = wrapper;
    fl_type_table[i].ptr.to = child;
    break;
  case FL_VECTOR:
    fl_type_table[i].of = wrapper;
    fl_type_table[i].vector.size = 0;
    fl_type_table[i].vector.to = child;
    break;
  default: { cg_error("(parser) fl_parser_get_typeid fail\n"); }
  }

  return i;
}

// transfer list ownership
size_t ts_struct_typeid(size_t* list, size_t length, fl_ast_t* decl) {
  size_t i;
  size_t j;

  string* id = decl->structure.id->identifier.string;

  for (i = 0; i < fl_type_size; ++i) {
    // struct and same length?
    if (fl_type_table[i].of == FL_STRUCT &&
        fl_type_table[i].structure.nfields == length) {
      if (0 == memcmp(list, fl_type_table[i].structure.fields,
                      sizeof(size_t) * length)) {
        free(list);

        ht_set(ts_hashtable, id->value, i);
        return i;
      }
    }
  }

  // add it!
  i = fl_type_size++;
  fl_type_table[i].of = FL_STRUCT;
  fl_type_table[i].id = id;
  fl_type_table[i].structure.decl = decl;
  fl_type_table[i].structure.fields = list;
  fl_type_table[i].structure.nfields = length;

  ht_set(ts_hashtable, id->value, i);
  return i;
}

size_t ts_named_typeid(string* id) { return ht_get(ts_hashtable, id->value); }

void ts_exit() {
  size_t i;

  for (i = 0; i < fl_type_size; ++i) {
    if (i < 13) {
      st_delete(&fl_type_table[i].id);
    }
    // struct and same length?
    if (fl_type_table[i].of == FL_STRUCT) {
      free(fl_type_table[i].structure.fields);
    }
  }

  free(fl_type_table);
  fl_type_table = 0;

  ht_free(ts_hashtable);
  ts_hashtable = 0;
}
