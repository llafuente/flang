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

ty_t* ts_type_table = 0;
size_t ts_type_size_s = 0;
ts_typeh_t* ts_hashtable = 0;

// 0 infer
// 1-12 built-in
// 13-x core
// x... user
void ts_init() {
  if (!ts_type_table) {
    ts_type_table = calloc(sizeof(ty_t), 100);

    // 0 means infer!
    ts_type_table[0].of = FL_INFER;
    // [1] void
    ts_type_table[TS_VOID].of = FL_VOID;
    ts_type_table[TS_VOID].id = st_newc("void", st_enc_ascii);

    // [2] bool
    ts_type_table[TS_BOOL].of = FL_NUMBER;
    ts_type_table[TS_BOOL].id = st_newc("bool", st_enc_ascii);
    ts_type_table[TS_BOOL].number.bits = 1;
    ts_type_table[TS_BOOL].number.fp = false;
    ts_type_table[TS_BOOL].number.sign = false;
    // [3-10] i8,u8,i16,u16,i32,u32,i64,u64
    size_t id = 2;
    size_t i = 3;
    char buffer[20];
    for (; i < 7; i++) {
      size_t bits = pow(2, i);
      ts_type_table[++id].of = FL_NUMBER;
      ts_type_table[id].number.bits = bits;
      ts_type_table[id].number.fp = false;
      ts_type_table[id].number.sign = false;
      sprintf(buffer, "u%zu", bits);
      ts_type_table[id].id = st_newc(buffer, st_enc_ascii);

      ts_type_table[++id].of = FL_NUMBER;
      ts_type_table[id].number.bits = bits;
      ts_type_table[id].number.fp = false;
      ts_type_table[id].number.sign = true;
      sprintf(buffer, "i%zu", bits);
      ts_type_table[id].id = st_newc(buffer, st_enc_ascii);
    }

    // [11] f32
    ts_type_table[++id].of = FL_NUMBER;
    ts_type_table[id].id = st_newc("f32", st_enc_ascii);
    ts_type_table[id].number.bits = 32;
    ts_type_table[id].number.fp = true;
    ts_type_table[id].number.sign = true;

    // [12] f64
    ts_type_table[++id].of = FL_NUMBER;
    ts_type_table[id].id = st_newc("f64", st_enc_ascii);
    ts_type_table[id].number.bits = 64;
    ts_type_table[id].number.fp = true;
    ts_type_table[id].number.sign = true;

    // [13] C-str (null-terminated)
    ts_type_table[++id].of = FL_POINTER;
    ts_type_table[id].id = st_newc("cstr", st_enc_ascii);
    ts_type_table[id].ptr.to = TS_I8;

    // [14] ptr void
    ts_type_table[++id].of = FL_POINTER;
    ts_type_table[id].id = st_newc("ptr<void>", st_enc_ascii);
    ts_type_table[id].ptr.to = TS_VOID;

    // adding types here, affects typesystem pass
    // because some are hardcoded atm!

    // [15+] core + user
    ts_type_size_s = ++id;
  }
}

bool ts_is_struct(size_t id) {
  ty_t t = ts_type_table[id];
  return t.of == FL_STRUCT;
}

bool ts_is_pointer(size_t id) {
  ty_t t = ts_type_table[id];
  return t.of == FL_POINTER;
}

size_t ts_get_pointer_level(size_t id) {
  size_t count = 0;
  ty_t t = ts_type_table[id];
  while (t.of == FL_POINTER) {
    t = ts_type_table[t.ptr.to];
    ++count;
  }
  return count;
}

bool ts_is_vector(size_t id) {
  ty_t t = ts_type_table[id];
  return t.of == FL_VECTOR;
}

bool ts_is_number(size_t id) {
  ty_t t = ts_type_table[id];
  return t.of == FL_NUMBER;
}

bool ts_is_fp(size_t id) {
  ty_t t = ts_type_table[id];
  return t.of == FL_NUMBER ? t.number.fp : false;
}

bool ts_is_int(size_t id) {
  ty_t t = ts_type_table[id];
  return t.of == FL_NUMBER ? !t.number.fp : false;
}

// TODO how to handle signed/unsigned, how's 'bigger'?
size_t ts_get_bigger_typeid(size_t a, size_t b) {
  ty_t t_a = ts_type_table[a];
  ty_t t_b = ts_type_table[b];

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

// TODO check for LIT_NUMERIC, and modify ty_id
// remove codegen side that do this right now
ast_t* ts_create_cast(ast_t* node, size_t type_id) {
  if (node->type == FL_AST_LIT_NUMERIC) {
    node->ty_id = type_id;
    return node;
  }

  ast_t* cast = (ast_t*)calloc(1, sizeof(ast_t));
  cast->token_start = 0;
  cast->token_end = 0;
  cast->type = FL_AST_CAST;
  cast->parent = node->parent;
  node->parent = cast;
  cast->cast.element = node;
  cast->ty_id = type_id;

  return cast;
}
ast_t* ts_create_left_cast(ast_t* parent, ast_t* left) {
  assert(parent->type == FL_AST_EXPR_BINOP);
  ast_t* cast = ts_create_cast(left, parent->ty_id);
  parent->binop.left = cast;

  return cast;
}

ast_t* ts_create_right_cast(ast_t* parent, ast_t* right) {
  assert(parent->type == FL_AST_EXPR_BINOP);
  ast_t* cast = ts_create_cast(right, parent->ty_id);
  parent->binop.right = cast;

  return cast;
}
void ts_cast_binop(ast_t* bo) {
  assert(bo->type == FL_AST_EXPR_BINOP);

  ast_t* l = bo->binop.left;
  ast_t* r = bo->binop.right;

  if (bo->ty_id != l->ty_id) {
    // cast left side
    ts_create_left_cast(bo, l);
  }

  if (bo->ty_id != r->ty_id) {
    ts_create_right_cast(bo, r);
  }
}

bool ts_pass_cb(ast_t* node, ast_t* parent, size_t level, void* userdata_in,
                void* userdata_out) {
  switch (node->type) {
  case FL_AST_STMT_RETURN: {
    ast_t* decl = node->parent;
    while (decl->parent && decl->type != FL_AST_DECL_FUNCTION) {
      decl = decl->parent;
    }

    if (decl->type != FL_AST_DECL_FUNCTION) {
      log_error("return statement found outside function scope");
    }

    size_t t = decl->func.ret_type->ty_id;
    if (t != node->ret.argument->ty_id) {
      ast_t* cast = ts_create_cast(node->ret.argument, t);
      node->ret.argument = cast;
    }
  } break;
  case FL_AST_LIT_STRING: {
    // TODO ptr<i8> atm -> string in the future
    node->ty_id = 16;
  } break;
  case FL_AST_LIT_IDENTIFIER: {
    if (node->identifier.resolve) {
      node->identifier.decl =
          ast_search_decl_var(node, node->identifier.string);

      if (node->parent->type == FL_AST_EXPR_CALL) {
        // see EXPR_CALL below
      } else if (node->parent->type == FL_AST_EXPR_MEMBER) {
        // it's handled below, but maybe can be optimized at this level...
      } else {
        // it's a var, copy type
        node->ty_id = ast_get_typeid(node->identifier.decl);
      }
    }
  } break;
  case FL_AST_EXPR_MEMBER: {
    ast_t* l = node->member.left;
    ast_t* p = node->member.property;

    size_t l_typeid;
    if (l->type == FL_AST_LIT_IDENTIFIER) {
      l->ty_id = ts_var_typeid(l);
    } else {
      ts_pass(l);
    }

    // now we should know left type
    // get poperty index -> typeid
    // TODO perf
    ty_t* type = &ts_type_table[l->ty_id];
    switch (type->of) {
    case FL_STRUCT: {
      node->ty_id = ts_struct_property_type(l->ty_id, p->identifier.string);
      node->member.idx = ts_struct_property_idx(l->ty_id, p->identifier.string);
    } break;
    case FL_POINTER: {
      node->ty_id = type->ptr.to;
      node->member.property = ts_create_cast(p, 9);
    } break;
    case FL_VECTOR: {
      node->ty_id = type->vector.to;
    } break;
    default: { log_error("invalid member access type"); }
    }

    if (ts_is_struct(l->ty_id)) {
    }

  } break;
  case FL_AST_EXPR_LUNARY: {
    switch (node->lunary.operator) {
    case FL_TK_EXCLAMATION:
      node->ty_id = 2; // bool
      break;
    case FL_TK_AND: {
      ast_t* el = node->lunary.element;
      ts_pass(el);
      node->ty_id = ts_wapper_typeid(FL_POINTER, el->ty_id);

      // ?node->ty_id = 13;
    } break;
    default:
      node->ty_id = node->lunary.element->ty_id;
    }
  } break;
  case FL_AST_EXPR_ASSIGNAMENT: {
    // ast_dump(node);

    ast_t* l = node->assignament.left;
    ast_t* r = node->assignament.right;

    ts_pass(l);
    ts_pass(r);

    size_t l_type = l->ty_id;
    size_t r_type = r->ty_id;

    if (l_type != r_type) {
      log_debug("assignament cast [%zu - %zu]", l_type, r_type);
      // TODO check cast if valid!

      node->assignament.right = ts_create_cast(r, l_type);
    }
    node->ty_id = l_type;
  } break;
  case FL_AST_EXPR_CALL: {
    size_t i;
    ast_t* args = node->call.arguments;
    ast_t* arg;
    size_t count = args->list.count;

    if (!node->ty_id) {
      // get types from arguments first
      for (i = 0; i < count; ++i) {
        ts_pass(args->list.elements[i]);
      }
      // now that we have our desired type
      // search for a compatible function
      string* callee = node->call.callee->identifier.string;

      ast_t* decl = ts_find_fn_decl(callee, args);
      if (!decl) {
        log_error("cannot find compatible function");
      }
      node->call.decl = decl;

      size_t cty_id = decl->ty_id;
      node->ty_id = ts_type_table[cty_id].func.ret;
      node->call.callee->ty_id = cty_id;
    }

    if (!node->call.callee->ty_id) {
      log_warning("ignore expr call type");
      break; // TODO passthought printf atm
    }

    ty_t* t = &ts_type_table[node->call.callee->ty_id];
    assert(t->of != FL_FUNCTION);

    ast_dump(node);

    // cast arguments
    for (i = 0; i < count; ++i) {
      arg = args->list.elements[i];

      // varargs goes raw!
      log_verbose("varargs[%d] params[%zu] i[%zu]", t->func.varargs,
                  t->func.nparams, i);

      if (t->func.varargs && t->func.nparams <= i) {
        break;
      }

      if (arg->ty_id != t->func.params[i]) {
        // cast right side
        args->list.elements[i] = ts_create_cast(arg, t->func.params[i]);
      }
    }

  } break;
  case FL_AST_EXPR_BINOP: {
    log_debug("binop found %d", node->binop.operator);
    ast_dump(node);
    // cast if necessary
    ast_t* l = node->binop.left;
    ast_t* r = node->binop.right;

    // operation that need casting or fp/int
    ts_pass(l);
    ts_pass(r);

    size_t l_type = l->ty_id;
    size_t r_type = r->ty_id;

    bool l_fp = ts_is_fp(l_type);
    bool r_fp = ts_is_fp(r_type);

    // binop
    switch (node->binop.operator) {
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
      ts_cast_binop(node);
    } break;
    case FL_TK_AND:
    case FL_TK_OR:
    case FL_TK_CARET:
    case FL_TK_LT2:
    case FL_TK_GT2:
      // left and right must be Integers!
      if (l_fp || r_fp) {
        log_error("invalid operants");
      }
    // fallthrough
    default: {
      bool l_static = ast_is_static(l);
      bool r_static = ast_is_static(r);

      if ((l_static && r_static) || (!l_static && !r_static)) {
        node->ty_id = ts_get_bigger_typeid(l_type, r_type);
        ts_cast_binop(node);
      } else if (l_static) {
        node->ty_id = r_type;
        ts_create_left_cast(node, l);
      } else if (r_static) {
        node->ty_id = l_type;
        ts_create_right_cast(node, r);
      }
    }
    }
  }
  }
  return true;
}

ast_t* ts_pass(ast_t* node) {
  log_debug("pass start!");

  ast_traverse(node, ts_pass_cb, 0, 0, 0, 0);
}

// wrapper types are
// * FL_POINTER
// * FL_VECTOR
size_t ts_wapper_typeid(ts_types_t wrapper, size_t child) {
  size_t i;

  for (i = 0; i < ts_type_size_s; ++i) {
    // TODO check length?!
    if (ts_type_table[i].of == wrapper && ts_type_table[i].ptr.to == child) {
      // if (wrapper != FL_VECTOR && ts_type_table[i].vector.length != 0)
      return i;
    }
  }
  // add it!
  i = ts_type_size_s++;
  switch (wrapper) {
  case FL_POINTER:
    ts_type_table[i].of = wrapper;
    ts_type_table[i].ptr.to = child;
    break;
  case FL_VECTOR:
    ts_type_table[i].of = wrapper;
    ts_type_table[i].vector.length = 0;
    ts_type_table[i].vector.to = child;
    break;
  default: { log_error("ts_wapper_typeid unhandled"); }
  }

  return i;
}

size_t ts_struct_property_idx(size_t id, string* property) {
  if (ts_type_table[id].of != FL_STRUCT) {
    log_error("type [%zu] is not an struct", id);
  }

  ast_t* list = ts_type_table[id].structure.decl->structure.fields;

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

  if (ts_type_table[id].of != FL_STRUCT) {
    log_error("type [%zu] is not an struct", id);
  }

  ast_t* list = ts_type_table[id].structure.decl->structure.fields;

  size_t i;
  for (i = 0; i < list->list.count; ++i) {
    if (st_cmp(list->list.elements[i]->field.id->identifier.string, property) ==
        0) {
      return ts_type_table[id].structure.fields[i];
    }
  }

  return 0;
}

void ts_named_set(string* id, ast_t* decl, size_t type_id) {
  ts_typeh_t* t = ts_named_type(id);

  if (!t) {
    t = (ts_typeh_t*)calloc(1, sizeof(ts_typeh_t));
    array_new(&t->list);

    strncpy(t->name, id->value, 64);

    HASH_ADD_STR(ts_hashtable, name, t);
  }

  array_append(&t->list, decl);
}

// transfer list ownership
size_t ts_struct_create(ast_t* decl) {
  size_t i;
  size_t j;
  ast_t* list = decl->structure.fields;
  size_t length = list->list.count;
  size_t* fields = calloc(length, sizeof(size_t));
  string* id = decl->structure.id->identifier.string;

  for (i = 0; i < length; ++i) {
    fields[i] = list->list.elements[i]->field.type->ty_id;
  }

  for (i = 0; i < ts_type_size_s; ++i) {
    // struct and same length?
    if (ts_type_table[i].of == FL_STRUCT &&
        ts_type_table[i].structure.nfields == length) {
      if (0 == memcmp(fields, ts_type_table[i].structure.fields,
                      sizeof(size_t) * length)) {
        free(fields);

        log_debug("SET type [%zu] = '%s'", i, id->value);
        ts_named_set(id, decl, i);
        return i;
      }
    }
  }

  // add it!
  i = ts_type_size_s++;
  ts_type_table[i].of = FL_STRUCT;
  ts_type_table[i].id = id;
  ts_type_table[i].structure.decl = decl;
  ts_type_table[i].structure.fields = fields;
  ts_type_table[i].structure.nfields = length;

  log_debug("SET type [%zu] = '%s'", i, id->value);
  ts_named_set(id, decl, i);
  return i;
}

FL_EXTERN size_t ts_struct_idx(ast_t* decl, string* id) {
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

size_t ts_fn_create(ast_t* decl) {
  string* id = decl->func.id->identifier.string;
  string* uid;

  // check for collisions
  if (ts_named_type(id)) {
    if (decl->func.uid) {
      if (ts_named_type(decl->func.uid)) {
        log_error("uid collision!");
      }
    } else {
      // create a unique name!
      uid = st_concat_random(id, 10);
      while (ts_named_type(uid)) {
        st_delete(&uid);
        uid = st_concat_random(id, 10);
      }
    }
    decl->func.uid = uid;
  } else {
    decl->func.uid = st_clone(id);
  }

  ast_t* params = decl->func.params;
  size_t length = params->list.count;
  size_t* tparams = calloc(length, sizeof(size_t));
  size_t ret = decl->func.ret_type->ty_id;
  size_t i;

  for (i = 0; i < length; ++i) {
    tparams[i] = params->list.elements[i]->ty_id;
  }

  for (i = 0; i < ts_type_size_s; ++i) {
    // function, same parameters length return type and varargs?
    if (ts_type_table[i].of == FL_FUNCTION &&
        ts_type_table[i].func.nparams == length &&
        ret == ts_type_table[i].func.ret &&
        ts_type_table[i].func.varargs == decl->func.varargs) {
      if (0 == memcmp(tparams, ts_type_table[i].func.params,
                      sizeof(size_t) * length)) {
        free(tparams);

        log_debug("SET fn type [%zu] = '%s'", i, id->value);
        ts_named_set(id, decl, i);
        return i;
      }
    }
  }

  // add it!
  i = ts_type_size_s++;
  ts_type_table[i].of = FL_FUNCTION;
  ts_type_table[i].id = id;
  ts_type_table[i].func.decl = decl;
  ts_type_table[i].func.params = tparams;
  ts_type_table[i].func.nparams = length;
  ts_type_table[i].func.ret = ret;
  ts_type_table[i].func.varargs = decl->func.varargs;

  log_debug("SET fn type [%zu] = '%s'", i, id->value);
  ts_named_set(id, decl, i);
  /*
  size_t t = ht_get(ts_hashtable, id->value);
  assert(t != i);
  */
  return i;
}

// TODO global functions!
size_t ts_fn_typeid(ast_t* id) {
  assert(id->type != FL_AST_LIT_IDENTIFIER);

  ast_t* fdecl = ast_find_fn_decl(id);

  // TODO search globals and assert!
  if (fdecl) {
    return fdecl->ty_id;
  }

  return 0;
}

// TODO handle args
ast_t* ts_find_fn_decl(string* id, ast_t* args_call) {
  array* arr = ast_find_fn_decls(args_call->parent, id);
  if (!arr) {
    log_error("undefined function: %s", id->value);
  }
  log_verbose("declarations with same name = %d\n", arr->size);

  if (arr->size == 1) {
    ast_t* ret = array_get(arr, 0);
    array_delete(arr);
    free(arr);
    return ret;
  }

  ast_t* decl;
  ast_t* params;
  ast_t* param;
  ast_t* ret_decl = 0;

  ast_t* arg_call;

  size_t i, j;
  size_t imax = args_call->list.count;
  size_t jmax = arr->size;

  for (j = 0; j < jmax; ++j) {
    decl = array_get(arr, j);
    params = decl->func.params;
    ast_dump(decl);
    // get types from arguments first

    for (i = 0; i < imax; ++i) {
      arg_call = args_call->list.elements[i];
      param = params->list.elements[i];
      ast_dump(arg_call);

      if (!arg_call->ty_id) {
        log_error("cannot find type of argument %zu", i);
      }

      if (!ts_castable(arg_call->ty_id, param->ty_id)) {
        break;
      }
      if (i == imax - 1) {
        // we reach the end all is ok!
        // this is compatible!
        ret_decl = decl;
      }
    }
  }

  array_delete(arr);
  free(arr);

  return ret_decl;
}

// TODO global vars!
size_t ts_var_typeid(ast_t* id) {
  assert(id->type != FL_AST_LIT_IDENTIFIER);

  ast_t* decl = ast_search_decl_var(id, id->identifier.string);

  if (!decl) {
    log_error("(ts) cannot find var declaration %s",
              id->identifier.string->value);
  }

  return ast_get_typeid(decl);
}

ts_typeh_t* ts_named_type(string* id) {
  ts_typeh_t* s;
  HASH_FIND_STR(ts_hashtable, id->value, s);

  return s;
}

size_t ts_named_typeid(string* id) {
  ts_typeh_t* s;
  HASH_FIND_STR(ts_hashtable, id->value, s);

  // TODO raise something ?!
  if (!s)
    return 0;

  if (s->list.size > 1) {
    log_error("not allowed?!");
  }

  ast_t* ast = (ast_t*)array_get(&s->list, 0);
  return ast->ty_id;
}

void ts_exit() {
  size_t i;

  for (i = 0; i < ts_type_size_s; ++i) {
    if (i < 13) {
      st_delete(&ts_type_table[i].id);
    }
    // struct and same length?
    if (ts_type_table[i].of == FL_STRUCT) {
      free(ts_type_table[i].structure.fields);
    } else if (ts_type_table[i].of == FL_FUNCTION) {
      free(ts_type_table[i].func.params);
    }
  }

  free(ts_type_table);
  ts_type_table = 0;

  ts_typeh_t* s;
  ts_typeh_t* tmp;
  HASH_ITER(hh, ts_hashtable, s, tmp) {
    HASH_DEL(ts_hashtable, s);
    array_delete(&s->list);
    free(s);
  }

  ts_hashtable = 0;
}
