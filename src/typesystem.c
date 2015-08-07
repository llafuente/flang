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

fl_type_t* fl_type_table = 0;
size_t fl_type_size = 0;
fl_type_cg_t* ts_hashtable = 0;

// 0 infer
// 1-12 built-in
// 13-x core
// x... user
void ts_init() {
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
      sprintf(buffer, "u%zu", bits);
      fl_type_table[id].id = st_newc(buffer, st_enc_ascii);

      fl_type_table[++id].of = FL_NUMBER;
      fl_type_table[id].number.bits = bits;
      fl_type_table[id].number.fp = false;
      fl_type_table[id].number.sign = true;
      sprintf(buffer, "i%zu", bits);
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

    // [13+] core + user
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
  log_debug("ts_get_bigger_typeid of %d & %d", t_a.of, t_b.of);

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
  case FL_AST_LIT_IDENTIFIER: {
    if (node->identifier.resolve) {
      if (node->parent->type == FL_AST_EXPR_CALL) {
        node->ty_id = ts_fn_typeid(node);
        node->parent->ty_id = fl_type_table[node->ty_id].func.ret;
      } else if (node->parent->type == FL_AST_EXPR_MEMBER) {
        // it's handled below, but maybe can be optimized at this level...
      } else {
        // it's a var
        node->ty_id = ts_var_typeid(node);
      }
    }
  } break;
  case FL_AST_EXPR_MEMBER: {
    fl_ast_t* l = node->member.left;
    fl_ast_t* p = node->member.property;

    size_t l_typeid;
    if (l->type == FL_AST_LIT_IDENTIFIER) {
      l->ty_id = ts_var_typeid(l);
    } else {
      ts_pass(l);
    }

    // now we should know left type
    // get poperty index -> typeid
    node->ty_id = ts_struct_property_type(l->ty_id, p->identifier.string);

  } break;
  case FL_AST_EXPR_ASSIGNAMENT: {
    // fl_ast_debug(node);

    fl_ast_t* l = node->assignament.left;
    fl_ast_t* r = node->assignament.right;

    if (l->type == FL_AST_LIT_IDENTIFIER) {
      l->ty_id = ts_var_typeid(l);
    } else {
      ts_pass(l);
    }
    ts_pass(r);

    size_t l_type = l->ty_id;
    size_t r_type = r->ty_id;

    if (l_type != r_type) {
      log_debug("assignament cast [%zu - %zu]", l_type, r_type);
      // TODO check cast if valid!

      CREATE_CAST(cast, r, l_type);

      node->assignament.right = cast;
    }
    node->ty_id = l_type;
  } break;
  case FL_AST_EXPR_CALL: {
    if (!node->ty_id) {
      ts_pass(node->call.callee);
    }

    if (!node->call.callee->ty_id) {
      log_warning("ignore expr call type");
      break; // TODO passthought printf atm
    }

    fl_type_t* t = &fl_type_table[node->call.callee->ty_id];
    assert(t->of != FL_FUNCTION);

    fl_ast_debug(node);

    // cast arguments
    fl_ast_t* args = node->call.arguments;
    fl_ast_t* arg;
    size_t i;
    size_t count = args->list.count;

    for (i = 0; i < count; ++i) {
      arg = args->list.elements[i];
      ts_pass(arg);

      if (arg->ty_id != t->func.params[i]) {
        // cast right side
        CREATE_CAST(cast, arg, t->func.params[i]);
        args->list.elements[i] = cast;
      }
    }

  } break;
  case FL_AST_EXPR_BINOP: {
    log_debug("binop found %d", node->binop.operator);
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
        log_error("invalid operants");
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
        CREATE_CAST(cast, node->binop.right, l_type);
        node->binop.right = cast;
        node->ty_id = l_type;
      } else {
        CREATE_CAST(cast, node->binop.left, r_type);
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
  log_debug("pass start!");

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
  default: { log_error("ts_wapper_typeid unhandled"); }
  }

  return i;
}

size_t ts_struct_property_idx(size_t id, string* property) {
  if (fl_type_table[id].of != FL_STRUCT) {
    log_error("type [%zu] is not an struct", id);
  }

  fl_ast_t* list = fl_type_table[id].structure.decl->structure.fields;

  size_t i;
  for (i = 0; i < list->list.count; ++i) {
    if (st_cmp(list->list.elements[i]->field.id->identifier.string, property) ==
        0) {
      return i;
    }
  }

  return -1;
}

size_t ts_struct_property_type(size_t id, string* property) {
  log_debug("ts_struct_property_type [%zu] '%s'", id, property->value);

  if (fl_type_table[id].of != FL_STRUCT) {
    log_error("type [%zu] is not an struct", id);
  }

  fl_ast_t* list = fl_type_table[id].structure.decl->structure.fields;

  size_t i;
  for (i = 0; i < list->list.count; ++i) {
    if (st_cmp(list->list.elements[i]->field.id->identifier.string, property) ==
        0) {
      return fl_type_table[id].structure.fields[i];
    }
  }

  return 0;
}

void ts_named_set(string* id, fl_ast_t* decl, size_t type_id) {
  fl_type_cg_t* t = (fl_type_cg_t*)malloc(sizeof(fl_type_cg_t));
  strncpy(t->name, id->value, 64);
  t->decl = decl;
  t->id = type_id;

  HASH_ADD_STR(ts_hashtable, name, t);
}

// transfer list ownership
size_t ts_struct_create(fl_ast_t* decl) {
  size_t i;
  size_t j;
  fl_ast_t* list = decl->structure.fields;
  size_t length = list->list.count;
  size_t* fields = calloc(length, sizeof(size_t));
  string* id = decl->structure.id->identifier.string;

  for (i = 0; i < length; ++i) {
    fields[i] = list->list.elements[i]->field.type->ty_id;
  }

  for (i = 0; i < fl_type_size; ++i) {
    // struct and same length?
    if (fl_type_table[i].of == FL_STRUCT &&
        fl_type_table[i].structure.nfields == length) {
      if (0 == memcmp(fields, fl_type_table[i].structure.fields,
                      sizeof(size_t) * length)) {
        free(fields);

        log_debug("SET type [%zu] = '%s'", i, id->value);
        ts_named_set(id, decl, i);
        return i;
      }
    }
  }

  // add it!
  i = fl_type_size++;
  fl_type_table[i].of = FL_STRUCT;
  fl_type_table[i].id = id;
  fl_type_table[i].structure.decl = decl;
  fl_type_table[i].structure.fields = fields;
  fl_type_table[i].structure.nfields = length;

  log_debug("SET type [%zu] = '%s'", i, id->value);
  ts_named_set(id, decl, i);
  return i;
}

size_t ts_fn_create(fl_ast_t* decl) {
  string* id = decl->func.id->identifier.string;
  fl_ast_t* params = decl->func.params;
  size_t length = params->list.count;
  size_t* tparams = calloc(length, sizeof(size_t));
  size_t ret = decl->func.ret_type->ty_id;
  size_t i;

  for (i = 0; i < length; ++i) {
    tparams[i] = params->list.elements[i]->ty_id;
  }

  for (i = 0; i < fl_type_size; ++i) {
    // function, same parameters length return type and varargs?
    if (fl_type_table[i].of == FL_FUNCTION &&
        fl_type_table[i].func.nparams == length &&
        ret == fl_type_table[i].func.ret &&
        fl_type_table[i].func.varargs == decl->func.varargs) {
      if (0 == memcmp(tparams, fl_type_table[i].func.params,
                      sizeof(size_t) * length)) {
        free(tparams);

        log_debug("SET fn type [%zu] = '%s'", i, id->value);
        ts_named_set(id, decl, i);
        return i;
      }
    }
  }

  // add it!
  i = fl_type_size++;
  fl_type_table[i].of = FL_FUNCTION;
  fl_type_table[i].id = id;
  fl_type_table[i].func.decl = decl;
  fl_type_table[i].func.params = tparams;
  fl_type_table[i].func.nparams = length;
  fl_type_table[i].func.ret = ret;
  fl_type_table[i].func.varargs = decl->func.varargs;

  log_debug("SET fn type [%zu] = '%s'", i, id->value);
  ts_named_set(id, decl, i);
  /*
  size_t t = ht_get(ts_hashtable, id->value);
  assert(t != i);
  */
  return i;
}

// TODO global functions!
size_t ts_fn_typeid(fl_ast_t* id) {
  assert(id->type != FL_AST_LIT_IDENTIFIER);

  fl_ast_t* fdecl = fl_ast_find_fn_decl(id);

  // TODO search globals and assert!
  if (fdecl) {
    return fdecl->ty_id;
  }

  return 0;
}

// TODO global vars!
size_t ts_var_typeid(fl_ast_t* id) {
  assert(id->type != FL_AST_LIT_IDENTIFIER);

  fl_ast_t* decl = fl_ast_search_decl_var(id, id->identifier.string);

  if (!decl) {
    log_error("(ts) cannot find var declaration %s",
              id->identifier.string->value);
  }

  return fl_ast_get_typeid(decl);
}

fl_type_cg_t* ts_named_type(string* id) {
  fl_type_cg_t* s;
  HASH_FIND_STR(ts_hashtable, id->value, s);

  return s;
}

size_t ts_named_typeid(string* id) {
  fl_type_cg_t* s;
  HASH_FIND_STR(ts_hashtable, id->value, s);

  return s ? s->id : 0;
}

void ts_exit() {
  size_t i;

  for (i = 0; i < fl_type_size; ++i) {
    if (i < 13) {
      st_delete(&fl_type_table[i].id);
    }
    // struct and same length?
    if (fl_type_table[i].of == FL_STRUCT) {
      free(fl_type_table[i].structure.fields);
    } else if (fl_type_table[i].of == FL_FUNCTION) {
      free(fl_type_table[i].func.params);
    }
  }

  free(fl_type_table);
  fl_type_table = 0;

  fl_type_cg_t* s;
  fl_type_cg_t* tmp;
  HASH_ITER(hh, ts_hashtable, s, tmp) {
    HASH_DEL(ts_hashtable, s);
    free(s);
  }

  ts_hashtable = 0;
}
