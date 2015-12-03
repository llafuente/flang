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

bool ty_is_struct(size_t id) {
  ty_t t = ts_type_table[id];
  return t.of == FL_STRUCT;
}

bool ty_is_vector(size_t id) {
  ty_t t = ts_type_table[id];
  return t.of == FL_VECTOR;
}

bool ty_is_number(size_t id) {
  ty_t t = ts_type_table[id];
  return t.of == FL_NUMBER;
}

bool ty_is_fp(size_t id) {
  ty_t t = ts_type_table[id];
  return t.of == FL_NUMBER ? t.number.fp : false;
}

bool ty_is_int(size_t id) {
  ty_t t = ts_type_table[id];
  return t.of == FL_NUMBER ? !t.number.fp : false;
}

bool ty_is_pointer(size_t id) {
  ty_t t = ts_type_table[id];
  return t.of == FL_POINTER;
}

size_t ty_get_pointer_level(size_t id) {
  size_t count = 0;
  ty_t t = ts_type_table[id];
  while (t.of == FL_POINTER) {
    t = ts_type_table[t.ptr.to];
    ++count;
  }
  return count;
}

bool ty_is_function(size_t id) {
  ty_t t = ts_type_table[id];
  return t.of == FL_FUNCTION;
}

// wrapper types are
// * FL_POINTER
// * FL_VECTOR
size_t ty_create_wrapped(ts_types_t wrapper, size_t child) {
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
  default: { log_error("ty_create_wrapped unhandled"); }
  }

  return i;
}

size_t ty_get_struct_prop_idx(size_t id, string* property) {
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

size_t ty_get_struct_prop_type(size_t id, string* property) {
  log_debug("ty_get_struct_prop_type [%zu] '%s'", id, property->value);

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

void ty_create_named(string* id, ast_t* decl, size_t type_id) {
  ts_type_hash_t* t = ty_get_type_by_name(id);

  if (!t) {
    t = (ts_type_hash_t*)calloc(1, sizeof(ts_type_hash_t));
    array_new(&t->list);

    strncpy(t->name, id->value, 64);

    HASH_ADD_STR(ts_hashtable, name, t);
  }

  array_append(&t->list, decl);
}

// transfer list ownership
size_t ty_create_struct(ast_t* decl) {
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
        ty_create_named(id, decl, i);
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
  ty_create_named(id, decl, i);
  return i;
}

size_t ty_create_fn(ast_t* decl) {
  string* id = decl->func.id->identifier.string;
  string* uid;

  // check for collisions
  if (ty_get_type_by_name(id)) {
    if (decl->func.uid) {
      if (ty_get_type_by_name(decl->func.uid)) {
        log_error("uid collision!");
      }
    } else {
      // create a unique name!
      uid = st_concat_random(id, 10);
      while (ty_get_type_by_name(uid)) {
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
        ty_create_named(id, decl, i);
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
  ty_create_named(id, decl, i);
  /*
  size_t t = ht_get(ts_hashtable, id->value);
  assert(t != i);
  */
  return i;
}

// TODO handle: global functions, collisions, etc.
size_t ty_get_fn_typeid(ast_t* id) {
  assert(id->type != FL_AST_LIT_IDENTIFIER);

  ast_t* fdecl = ast_find_fn_decl(id);

  // TODO search globals and assert!
  if (fdecl) {
    return fdecl->ty_id;
  }

  return 0;
}

ts_type_hash_t* ty_get_type_by_name(string* id) {
  ts_type_hash_t* s;
  HASH_FIND_STR(ts_hashtable, id->value, s);

  return s;
}

size_t ty_get_typeid_by_name(ast_t* node) {
  assert(node->type == FL_AST_LIT_IDENTIFIER);

  string* id = node->identifier.string;

  ts_type_hash_t* s;
  HASH_FIND_STR(ts_hashtable, id->value, s);

  // TODO raise something ?!
  if (!s)
    return 0;

  if (s->list.size > 1) {
    ast_raise_error(node, "Found many types with the same name");
  }

  ast_t* ast = (ast_t*)array_get(&s->list, 0);
  return ast->ty_id;
}
