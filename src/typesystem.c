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
