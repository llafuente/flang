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

    // adding types here
    // affects: typesystem pass (some are hardcoded)
    // also affects: ts_exit st_delete(xx.id)

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

bool ts_is_function(size_t id) {
  ty_t t = ts_type_table[id];
  return t.of == FL_FUNCTION;
}

// only promote numbers
size_t ts_promote_typeid(size_t a, size_t b) {
  ty_t t_a = ts_type_table[a];
  ty_t t_b = ts_type_table[b];

  assert(t_a.of == FL_NUMBER);
  assert(t_b.of == FL_NUMBER);

  // check floating point
  if (t_a.number.fp && !t_b.number.fp) {
    return a;
  }

  if (t_b.number.fp && !t_a.number.fp) {
    return b;
  }
  // check different sign
  if (!t_b.number.sign && t_a.number.sign) {
    return a;
  }

  if (t_b.number.sign && !t_a.number.sign) {
    return b;
  }

  // check bits
  return t_a.number.bits > t_b.number.bits ? a : b;
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
    decl->func.uid = st_clone(id); // TODO this should be uid ?
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
    log_verbose("undefined function: '%s' must be a variable", id->value);
    ast_t* decl = ast_search_id_decl(args_call->parent, id);
    if (!decl) {
      log_error("no function/var: '%s'", id->value);
    }
    // now search any function that has that ty_id
    // type is pointer to function so
    size_t fn_ty = ts_type_table[decl->ty_id].ptr.to;
    return ts_type_table[fn_ty].func.decl;
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
    assert(decl->type == FL_AST_DECL_FUNCTION);

    params = decl->func.params;
    // get types from arguments first

    for (i = 0; i < imax; ++i) {
      arg_call = args_call->list.elements[i];
      param = params->list.elements[i];

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
  assert(id->type == FL_AST_LIT_IDENTIFIER);
  log_verbose("%s", id->identifier.string->value)

      ast_t* decl = ast_search_id_decl(id, id->identifier.string);

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
    if (i < 15) {
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
