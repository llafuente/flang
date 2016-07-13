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

#include "flang/common.h"
#include "flang/libts.h"
#include "flang/debug.h"
#include "flang/libast.h"

bool ty_is_struct(u64 id) {
  ty_t t = ts_type_table[id];
  return t.of == FL_STRUCT;
}

bool ty_is_vector(u64 id) {
  ty_t t = ts_type_table[id];
  return t.of == FL_VECTOR;
}

bool ty_is_number(u64 id) {
  ty_t t = ts_type_table[id];
  return t.of == FL_NUMBER;
}

bool ty_is_fp(u64 id) {
  ty_t t = ts_type_table[id];
  return t.of == FL_NUMBER ? t.number.fp : false;
}

bool ty_is_int(u64 id) {
  ty_t t = ts_type_table[id];
  return t.of == FL_NUMBER ? !t.number.fp : false;
}

bool ty_is_pointer(u64 id) {
  ty_t t = ts_type_table[id];
  return t.of == FL_POINTER;
}

bool ty_is_template(u64 id) {
  ty_t t = ts_type_table[id];
  return t.of == FL_TEMPLATE;
}

u64 ty_get_pointer_level(u64 id) {
  u64 count = 0;
  ty_t t = ts_type_table[id];
  while (t.of == FL_POINTER) {
    t = ts_type_table[t.ptr.to];
    ++count;
  }
  return count;
}

bool ty_is_function(u64 id) {
  ty_t t = ts_type_table[id];
  return t.of == FL_FUNCTION;
}

// wrapper types are
// * FL_POINTER
// * FL_VECTOR
u64 ty_create_wrapped(ts_types_t wrapper, u64 child) {
  u64 i;

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

u64 ty_get_struct_prop_idx(u64 id, string* property) {
  if (ts_type_table[id].of != FL_STRUCT) {
    log_error("type [%zu] is not an struct", id);
  }

  ty_t t = ts_type_table[id];
  string** properties = t.structure.properties;

  u64 i;
  for (i = 0; i < t.structure.nfields; ++i) {
    if (st_cmp((const string*)properties[i], property) == 0) {
      return i;
    }
  }

  return -1;
}

u64 ty_get_struct_prop_type(u64 id, string* property) {
  log_debug("ty_get_struct_prop_type [%zu] '%s'", id, property->value);

  if (ts_type_table[id].of != FL_STRUCT) {
    log_error("type [%zu] is not an struct", id);
  }

  ty_t t = ts_type_table[id];
  string** properties = t.structure.properties;

  u64 i;
  for (i = 0; i < t.structure.nfields; ++i) {
    if (st_cmp((const string*)properties[i], property) == 0) {
      return t.structure.fields[i];
    }
  }

  return 0;
}

bool __fn_collision(ast_t* where, ast_t* scope, char* ty_name) {
  ast_t* redef;
  do {
    // a function cannot collide with a variable
    redef = (ast_t*)hash_get(scope->block.variables, ty_name);
    if (redef) {
      ast_raise_error(
          where,
          "Function name '%s' in use by a variable, previously defined at %s",
          ty_name, ast_get_location(redef)->value);
    }

    // a function cannot collide with a struct
    redef = (ast_t*)hash_get(scope->block.types, ty_name);
    if (redef && redef->type != AST_DECL_FUNCTION) {
      ast_raise_error(
          where,
          "Function name '%s' in use by a type, previously defined at %s",
          ty_name, ast_get_location(redef)->value);
    }
  } while (scope->block.scope != AST_SCOPE_GLOBAL &&
           (scope = ast_get_scope(scope)));
  return false;
}

bool __param_collision(ast_t* where, ast_t* scope, char* ty_name) {
  ast_t* redef;
  do {
    // a parameter cannot collide with a variable
    redef = (ast_t*)hash_get(scope->block.variables, ty_name);
    if (redef) {
      ast_raise_error(
          where,
          "Parameter name '%s' in use by a variable, previously defined at %s",
          ty_name, ast_get_location(redef)->value);
      return true;
    }

    // a parameter cannot collide with a type
    redef = (ast_t*)hash_get(scope->block.types, ty_name);
    if (redef) {
      ast_raise_error(
          where,
          "Parameter name '%s' in use by a type, previously defined at %s",
          ty_name, ast_get_location(redef)->value);
      return true;
    }
  } while (scope->block.scope != AST_SCOPE_GLOBAL &&
           (scope = ast_get_scope(scope)));

  return false;
}

bool __struct_collision(ast_t* where, ast_t* scope, char* ty_name) {
  ast_t* redef;
  do {
    // a function cannot collide with a variable
    redef = (ast_t*)hash_get(scope->block.variables, ty_name);
    if (redef) {
      ast_raise_error(
          where,
          "Type name '%s' in use by a variable, previously defined at %s",
          ty_name, ast_get_location(redef)->value);
    }

    // a function cannot collide with a struct
    redef = (ast_t*)hash_get(scope->block.types, ty_name);
    if (redef) {
      ast_raise_error(
          where,
          "Type name '%s' in use by another type, previously defined at %s",
          ty_name, ast_get_location(redef)->value);
    }
  } while (scope->block.scope != AST_SCOPE_GLOBAL &&
           (scope = ast_get_scope(scope)));

  return false;
}

// transfer list ownership
u64 ty_create_struct(ast_t* decl) {
  u64 i;
  u64 j;
  ast_t* list = decl->structure.fields;
  u64 length = list->list.count;
  u64* fields = calloc(length, sizeof(u64));
  string** properties = calloc(length, sizeof(string*));
  string* id = decl->structure.id->identifier.string;

  ast_t** elements = list->list.elements;
  for (i = 0; i < length; ++i) {
    fields[i] = elements[i]->field.type->ty_id;
    properties[i] = elements[i]->field.id->identifier.string;
  }

  // add it!
  i = ts_type_size_s++;
  ts_type_table[i].of = FL_STRUCT;
  ts_type_table[i].id = id;
  ts_type_table[i].structure.decl = decl;
  ts_type_table[i].structure.fields = fields;
  ts_type_table[i].structure.properties = properties;
  ts_type_table[i].structure.nfields = length;

  log_debug("type register (struct) id='%s' ty=%zu", id->value, i);

  // search nearest scope and add it
  ast_t* x = ast_get_scope(decl);
  if (!__struct_collision(decl, x, id->value)) {
    // TODO review casting, it works, but it's ok ?
    hash_set(x->block.types, id->value, decl);
    return i;
  }

  return 0;
}

bool ty_compatible_fn(u64 ty_id, ast_t* arg_list, bool strict,
                      bool template) {
  log_silly("fn ty_id %zu, strict? %d, template? %d", ty_id, strict, template);
  ty_t at = ts_type_table[ty_id];

  // TODO assert?
  if (at.of != FL_FUNCTION) {
    return false;
  }
  // function, same parameters length return type and varargs?
  u64 i;
  u64 current;
  u64 expected;
  for (i = 0; i < arg_list->list.count; ++i) {
    // end reached it's compatible, the rest is varargs
    if (at.func.varargs && i == at.func.nparams)
      break;

    current = arg_list->list.elements[i]->ty_id;
    expected = at.func.params[i];

    // strict - same type
    if (strict && current != expected) {
      log_silly("(strict) parameter %zu not compatible %zu != %zu", i, current,
                expected) return false;
    }

    ty_t at2 = ts_type_table[expected];
    if (at2.of == FL_TEMPLATE) {
      if (template) {
        log_silly("(template) parameter %zu use", i) continue;
      }
      log_silly("(template) parameter %zu reject", i) return false;
    } else if (!ts_castable(current, expected)) {
      log_silly("(cast) parameter %zu cannot be casted", i);
      return false;
    }
  }
  return true;
}
bool ty_compatible_struct(u64 a, u64 b) {
  ty_t at = ts_type_table[a];
  ty_t bt = ts_type_table[b];
  assert(at.of == FL_STRUCT);
  assert(bt.of == FL_STRUCT);

  // must be <= in length
  // if (at.structure.nfields > bt.structure.nfields) {
  //  return false;
  //}
  u64 i;
  for (i = 0; i < at.structure.nfields; ++i) {
    // reach the end of b, and all is compatible!
    if (i == bt.structure.nfields) {
      return true;
    }

    if (at.structure.fields[i] != bt.structure.fields[i]) {
      return false;
    }
  }

  return true;
}

u64 ty_create_fn(ast_t* decl) {
  string* id = decl->func.id->identifier.string;
  char* cstr = id->value;
  string* uid;

  ast_t* rscope = ast_get_global_scope(decl);

  ast_t* t = hash_get(rscope->block.uids, cstr);

  // check for collisions
  if (t != 0) {
    // we force an uid?
    if (decl->func.uid) {
      t = hash_get(rscope->block.uids, decl->func.uid->value);
      if (t != 0) {
        ast_raise_error(decl,
                        "Function #id collision found, previously used at %s",
                        ast_get_location(t));
      }
    } else {
      // create a unique name!
      uid = st_concat_random(id, 10);
      t = hash_get(rscope->block.uids, uid->value);
      while (t != 0) {
        st_delete(&uid);
        uid = st_concat_random(id, 10);
      }
      decl->func.uid = uid;
    }
    // !t && !uid ?
  } else if (!decl->func.uid) {
    decl->func.uid = st_clone(id);
  }

  assert(decl->func.uid != 0);

  ast_t* params = decl->func.params;
  u64 length = params->list.count;
  u64* tparams = calloc(length, sizeof(u64));
  u64 ret = decl->func.ret_type->ty_id;
  u64 i;

  for (i = 0; i < length; ++i) {
    tparams[i] = params->list.elements[i]->ty_id;
    if (ts_type_table[tparams[i]].of == FL_TEMPLATE) {
      decl->func.templated = true;
    }
  }

  // add it!
  u64 ty_id = ts_type_size_s++;
  ts_type_table[ty_id].of = FL_FUNCTION;
  ts_type_table[ty_id].id = id;
  ts_type_table[ty_id].func.decl = decl;
  ts_type_table[ty_id].func.params = tparams;
  ts_type_table[ty_id].func.nparams = length;
  ts_type_table[ty_id].func.ret = ret;
  ts_type_table[ty_id].func.varargs = decl->func.varargs;

  ast_t* attach_to;
  ast_t* from;
  attach_to = ast_get_scope(decl);

  if (!__fn_collision(decl, attach_to, id->value)) {
    array* lfunc = hash_get(attach_to->block.functions, cstr);
    log_debug("type register (fn) id='%s' ty=%zu @[%p]", cstr, i, lfunc);

    if (!lfunc) {
      lfunc = pool_new(sizeof(array));
      array_new(lfunc);
      hash_set(attach_to->block.functions, cstr, lfunc);
    }
    array_append(lfunc, decl);
    hash_set(attach_to->block.types, cstr, decl);

    hash_set(rscope->block.uids, decl->func.uid->value, decl);

    // check param names don't collide
    if (!decl->func.ffi && !decl->func.templated) {
      ast_t* p;
      ast_t* body = decl->func.body;

      for (i = 0; i < length; ++i) {
        p = params->list.elements[i];
        if (__param_collision(p, body, p->param.id->identifier.string->value)) {
          return 0;
        } else {
          hash_set(body->block.variables, p->param.id->identifier.string->value,
                   p);
        }
      }
    }

    return ty_id;
  }

  return 0;
}

// transfer list ownership
void ty_create_var(ast_t* decl) {
  assert(decl->type == AST_DTOR_VAR);

  char* cstr = decl->var.id->identifier.string->value;
  ast_t* attach_to;
  ast_t* from;
  switch (decl->var.scope) {
  case AST_SCOPE_BLOCK:
    from = attach_to = ast_get_scope(decl);
    break;
  case AST_SCOPE_GLOBAL:
    from = attach_to = ast_get_global_scope(decl);
    break;
  default: {} // avoid warning
  }
  // redefinition?
  ast_t* redef;
  do {
    if (from->block.scope != AST_SCOPE_GLOBAL) {
      from = ast_get_scope(from);
    }

    redef = (ast_t*)hash_get(from->block.variables, cstr);
    if (redef) {
      ast_raise_error(redef,
                      "Variable '%s' redefinition, previously defined at %s",
                      cstr, ast_get_location(redef)->value);
    }

    redef = (ast_t*)hash_get(from->block.types, cstr);
    if (redef) {
      ast_raise_error(
          redef,
          "Variable name '%s' in use by a type, previously defined at %s", cstr,
          ast_get_location(redef)->value);
    }

  } while (from->block.scope != AST_SCOPE_GLOBAL);

  hash_set(attach_to->block.variables, cstr, decl);
}

// templates are registered inside the block
// always get a new id, be cautious atm
u64 ty_create_template(ast_t* decl) {
  string* realname = st_newc("$", st_enc_utf8);
  st_append(&realname, decl->tpl.id->identifier.string);

  // add it!
  u64 ty_id = ts_type_size_s++;
  ts_type_table[ty_id].of = FL_TEMPLATE;
  ts_type_table[ty_id].id = realname;
  ts_type_table[ty_id].tpl.decl = decl;

  ast_t* attach_to = ast_get_scope(decl);
  hash_set(attach_to->block.types, realname->value, decl);

  return ty_id;
}
