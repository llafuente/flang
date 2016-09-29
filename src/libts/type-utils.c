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
#include "flang/libts.h"
#include "flang/debug.h"
#include "flang/libast.h"

ty_t ty(u64 ty_id) {
  // TODO check out-of-bounds
  return ts_type_table[ty_id];
}

bool ty_is_struct(u64 id) { return ty(id).of == TY_STRUCT; }

bool ty_is_vector(u64 id) { return ty(id).of == TY_VECTOR; }

bool ty_is_number(u64 id) { return ty(id).of == TY_NUMBER; }

bool ty_is_fp(u64 id) {
  ty_t t = ty(id);
  return t.of == TY_NUMBER ? t.number.fp : false;
}

bool ty_is_int(u64 id) {
  ty_t t = ty(id);
  return t.of == TY_NUMBER ? !t.number.fp : false;
}

bool ty_is_pointer(u64 id) {
  ty_t t = ty(id);
  return t.of == TY_POINTER;
}

bool ty_is_reference(u64 id) {
  ty_t t = ty(id);
  return t.of == TY_REFERENCE;
}

bool ty_is_pointer_like(u64 id) {
  ty_t t = ty(id);
  return t.of == TY_POINTER || t.of == TY_REFERENCE;
}

bool ty_is_template(u64 id) { return ty(id).of == TY_TEMPLATE; }

u64 ty_get_pointer_level(u64 id) {
  u64 count = 0;
  ty_t t = ty(id);
  while (t.of == TY_POINTER) {
    t = ty(t.ptr.to);
    ++count;
  }
  return count;
}

bool ty_is_function(u64 id) { return ty(id).of == TY_FUNCTION; }

bool ty_is_templated(u64 id) {
  ty_t t = ty(id);

  if (t.of == TY_POINTER || t.of == TY_REFERENCE) {
    return ty_is_templated(t.ptr.to);
  }

  return (t.of == TY_TEMPLATE) || t.templated;
}

// wrapper types are
// * TY_POINTER
// * TY_VECTOR
u64 ty_create_wrapped(ts_types_t wrapper, u64 child) {
  u64 i;
  ty_t t;
  for (i = 0; i < ts_type_size_s; ++i) {
    // TODO check length?!
    t = ty(i);
    if (t.of == wrapper && t.ptr.to == child) {
      // if (wrapper != TY_VECTOR && ts_type_table[i].vector.length != 0)
      return i;
    }
  }
  // add it!
  i = ts_type_size_s++;
  switch (wrapper) {
  case TY_POINTER:
    ts_type_table[i].of = wrapper;
    ts_type_table[i].ptr.to = child;
    break;
  case TY_VECTOR:
    ts_type_table[i].of = wrapper;
    ts_type_table[i].vector.length = 0;
    ts_type_table[i].vector.to = child;
    break;
  case TY_REFERENCE:
    ts_type_table[i].of = wrapper;

    if (ts_type_table[child].of == wrapper) {
      ast_raise_error(0, "Cannot create a reference of a reference.");
    }

    ts_type_table[i].ref.to = child;
    break;
  default: { log_error("ty_create_wrapped unhandled"); }
  }

  ts_type_table[i].templated = ty_is_templated(child);

  return i;
}

u64 ty_get_struct_prop_idx(u64 id, string* property) {
  ty_t t = ty(id);

  if (t.of != TY_STRUCT) {
    log_error("type [%zu] is not an struct", id);
  }

  array* props = (array*)&t.structure.members;
  void** properties = props->values;

  // check properties first
  u64 i;
  for (i = 0; i < props->length; ++i) {
    if (st_cmp((const string*)properties[i], property) == 0) {
      return i;
    }
  }

  // check aliases
  array* aliases = (array*)&t.structure.alias;
  properties = props->values;
  ts_type_struct_alias_t* alias;
  for (i = 0; i < aliases->length; ++i) {
    alias = aliases->values[i];
    if (st_cmp((const string*)alias->name, property) == 0) {
      return alias->index;
    }
  }
  return -1;
}

u64 ty_get_struct_prop_type(u64 id, string* property) {
  log_debug("ty_get_struct_prop_type [%zu] '%s'", id, property->value);

  ty_t t = ty(id);

  if (t.of != TY_STRUCT) {
    log_error("type [%zu] is not an struct", id);
  }

  u64 index = ty_get_struct_prop_idx(id, property);
  if (index == -1)
    return 0;

  return t.structure.fields[index];
}

bool __fn_collision(ast_t* where, ast_t* scope, char* ty_name, char* uid_name) {
  log_debug("check collisions id='%s' uid='%s'", ty_name, uid_name);
  ast_t* redef;

  bool same_name = strcmp(ty_name, uid_name) == 0;

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
    if (!same_name) {
      // if my UID != ID, i must not collide with functions
      redef = (ast_t*)hash_get(scope->block.functions, uid_name);
      if (redef) {
        ast_raise_error(where, "Function UID '%s' in use by other function, "
                               "previously defined at %s",
                        uid_name, ast_get_location(redef)->value);
      }
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

void __ty_template_usedby(u64 id, ast_t* decl) {
  ty_dump_table();
  log_silly("set: %lu %p", id, decl);

  ty_t type = ty(id);

  switch (type.of) {
  case TY_VOID:
  case TY_NUMBER:
  case TY_INFER:
    // ignore those types, appears when looping struct/functions
    break;
  case TY_REFERENCE:
  case TY_POINTER:
  case TY_VECTOR:
    __ty_template_usedby(type.ptr.to, decl);
    break;
  case TY_STRUCT: {
    // for each type
    u64 max = type.structure.members.length;

    // check properties first
    u64 i;
    for (i = 0; i < max; ++i) {
      __ty_template_usedby(type.structure.fields[i], decl);
    }
  } break;
  case TY_FUNCTION: {
    // for each type
    u64 max = type.func.nparams;

    // check properties first
    u64 i;
    for (i = 0; i < max; ++i) {
      __ty_template_usedby(type.func.params[i], decl);
    }
  } break;
  case TY_TEMPLATE:
    // add it!
    array_push(&type.tpl.usedby, decl);
    log_silly("push! %lu", type.tpl.usedby.length);
  }
}

// transfer list ownership
u64 ty_create_struct(ast_t* decl) {
  fl_assert(decl->structure.id != 0);

  u64 i;
  u64 j;
  ast_t* list = decl->structure.fields;
  u64 length = list->list.length;
  u64* fields = calloc(length, sizeof(u64));
  array properties;
  array alias;
  array_new(&properties);
  array_new(&alias);
  string* id = decl->structure.id->identifier.string;

  int templates = 0;

  ast_t** elements = list->list.values;
  // field pass
  for (i = 0; i < length; ++i) {
    if (elements[i]->type == AST_DECL_STRUCT_FIELD) {
      fields[i] = elements[i]->field.type->ty_id;
      if (ty_is_templated(fields[i])) {
        ++templates;
        __ty_template_usedby(fields[i], decl);
      }
      array_push(&properties, elements[i]->field.id->identifier.string);
    }
  }

  if (templates) {
    log_silly("has templates");
    decl->structure.templated = true;
  }

  // alias pass
  for (i = 0; i < length; ++i) {
    if (elements[i]->type == AST_DECL_STRUCT_ALIAS) {
      ts_type_struct_alias_t* als = pool_new(sizeof(ts_type_struct_alias_t));
      als->name = elements[i]->alias.name->identifier.string;
      als->id = elements[i]->alias.id->identifier.string;
      for (int j = 0; j < properties.length; ++j) {
        if (st_cmp(properties.values[j], als->id) == 0) {
          als->index = j;
          elements[i]->alias.id->ty_id = elements[i]->alias.name->ty_id =
              elements[i]->ty_id = fields[j];
          // fail later!
        }
      }

      array_push(&alias, als); // add the AST directly
                               // calc type
    }
  }

  // check for a struct with the same properties / types / alias
  int same_struct_found = -1;
  ty_t type;
  if (!templates) {
    for (i = 0; i < ts_type_size_s; ++i) {
      type = ty(i);

      if (type.of == TY_STRUCT &&
          type.structure.members.length == properties.length) {
        bool same_props = true;
        for (int j = 0; j < properties.length; ++j) {
          if (fields[j] != type.structure.fields[j] ||
              st_cmp(properties.values[j], type.structure.members.values[j]) !=
                  0) {
            same_props = false;
            break;
          }
        }

        if (same_props) {
          free(fields);
          array_delete(&properties);
          array_delete(&alias);
          same_struct_found = i;
          log_debug("type register (struct) same prop found on %lu", i);
          break;
        }
      }
    }
  }

  if (same_struct_found == -1 || templates) {
    // add it!
    string* s = st_newc("array_$tpl", st_enc_utf8);
    if (st_cmp(s, id) == 0) {
      fl_assert(false);
    }

    i = ts_type_size_s++;
    ts_type_table[i].of = TY_STRUCT;
    ts_type_table[i].id = id;
    ts_type_table[i].templated = templates > 0;
    ts_type_table[i].structure.decl = decl;
    ts_type_table[i].structure.fields = fields;
    ts_type_table[i].structure.members = properties;
    ts_type_table[i].structure.alias = alias;
  } else {
    i = same_struct_found;
  }

  log_debug("type register (struct) id='%s' ty=%zu", id->value, i);
  // even with ty_id uniques, we need to declare the struct in the scope
  // search nearest scope and add it
  ast_t* x = ast_get_scope(decl);
  if (!__struct_collision(decl, x, id->value)) {
    // TODO review casting, it works, but it's ok ?
    hash_set(x->block.types, id->value, decl);
    return i;
  }

  return 0;
}

bool ty_compatible_fn(u64 ty_id, ast_t* arg_list, bool strict, bool template) {
  log_silly("fn ty_id %zu, strict? %d, template? %d", ty_id, strict, template);
  ty_t at = ty(ty_id);

  fl_assert(ty_id > 0);
  fl_assert(arg_list->type == AST_LIST);
  fl_assert(at.of == TY_FUNCTION);

  // function, same parameters length return type and varargs?
  u64 i;
  u64 current;
  u64 expected;
  u64 args_sent = arg_list->list.length;
  u64 args_expected = at.func.nparams;
  ast_t* arg;

  if (!at.func.varargs && args_sent != args_expected) {
    return false;
  }

  for (i = 0; i < args_sent; ++i) {
    log_silly("parameter %zu of %zu/%zu", i, arg_list->list.length,
              at.func.nparams);

    // end reached it's compatible, the rest is varargs
    if (at.func.varargs && i == at.func.nparams)
      break;
    arg = arg_list->list.values[i];
    current = arg->ty_id;
    expected = at.func.params[i];

    // strict - same type
    if (strict && current != expected) {
      log_silly("(strict) parameter %zu not compatible %zu != %zu", i, current,
                expected);
      return false;
    }

    ty_t at2 = ty(expected);
    if (at2.templated) {
      if (template) {
        log_silly("(template) parameter %zu use", i) continue;
      }
      log_silly("(template) parameter %zu reject", i) return false;
    } else {
      ts_cast_modes_t cm = ts_cast_mode(current, expected);
      // NOTE CAST_EXPLICIT it a not compatible type
      // this is called by templates, if we cast down number
      // we broke the template purpose so it's invalid
      if (cm == CAST_INVALID) {
        log_silly("(cast) parameter %zu cast invalid", i);
        return false;
      }

      if (cm == CAST_EXPLICIT && !ast_is_literal(arg)) {
        log_silly("(cast) parameter %zu cast explicit and not literal", i);
        return false;
      }
    }
  }
  return true;
}

bool ty_compatible_struct(u64 a, u64 b) {
  ty_t at = ty(a);
  ty_t bt = ty(b);
  fl_assert(at.of == TY_STRUCT);
  fl_assert(bt.of == TY_STRUCT);

  // must be <= in length
  // if (at.structure.nfields > bt.structure.nfields) {
  //  return false;
  //}
  u64 i;
  for (i = 0; i < at.structure.members.length; ++i) {
    // reach the end of b, and all is compatible!
    if (i == bt.structure.members.length) {
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
  char* fn_id = id->value;
  string* uid;

  ast_t* rscope = ast_get_global_scope(decl);

  ast_t* t = hash_get(rscope->block.uids, fn_id);

  // check for collisions
  if (t != 0) {
    // we force an uid?
    if (decl->func.uid) {
      t = hash_get(rscope->block.uids, decl->func.uid->value);
      if (t != 0) {
        // ast_dump_s(t);
        ast_raise_error(
            decl,
            "Function #id collision found for '%s', previously used at %s",
            decl->func.uid->value, ast_get_location(t)->value);
        return 0;
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

  char* fn_uid = decl->func.uid->value;

  fl_assert(decl->func.uid != 0);

  ast_t* params = decl->func.params;
  u64 length = params->list.length;
  u64* tparams = calloc(length, sizeof(u64));
  u64 ret = decl->func.ret_type->ty_id;
  u64 i;
  int templates = 0;

  for (i = 0; i < length; ++i) {
    tparams[i] = params->list.values[i]->ty_id;
    if (ty_is_templated(tparams[i])) {
      ++templates;
      __ty_template_usedby(tparams[i], decl);
    }

    log_silly("param %s %lu is ty_id = %lu", fn_uid, i, tparams[i])
  }

  if (templates) {
    decl->func.templated = true;
  }

  // add it!
  u64 ty_id = ts_type_size_s++;
  ts_type_table[ty_id].of = TY_FUNCTION;
  ts_type_table[ty_id].id = id;
  ts_type_table[ty_id].templated = templates > 0;
  ts_type_table[ty_id].func.decl = decl;
  ts_type_table[ty_id].func.params = tparams;
  ts_type_table[ty_id].func.nparams = length;
  ts_type_table[ty_id].func.ret = ret;
  ts_type_table[ty_id].func.varargs = decl->func.varargs;

  ast_t* attach_to;
  ast_t* from;
  attach_to = ast_get_scope(decl);

  if (!__fn_collision(decl, attach_to, fn_id, fn_uid)) {
    array* lfunc = hash_get(attach_to->block.functions, fn_id);
    log_debug("type register (fn) id='%s' uid='%s' ty=%zu @[%p]", fn_id, fn_uid,
              ty_id, lfunc);

    // define the function with ID
    if (!lfunc) {
      lfunc = pool_new(sizeof(array));
      array_new(lfunc);
      hash_set(attach_to->block.functions, fn_id, lfunc);
    }
    array_push(lfunc, decl);
    // define the function with UID
    lfunc = pool_new(sizeof(array));
    array_new(lfunc);
    array_push(lfunc, decl);
    hash_set(attach_to->block.functions, fn_uid, lfunc);

    hash_set(attach_to->block.types, fn_uid, decl);

    hash_set(rscope->block.uids, fn_uid, decl);

    // check param names don't collide
    if (!decl->func.ffi && !decl->func.templated) {
      ast_t* p;
      ast_t* body = decl->func.body;

      for (i = 0; i < length; ++i) {
        p = params->list.values[i];
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
  fl_assert(decl->type == AST_DTOR_VAR);

  // do not attach the same variable to a scope many times
  // register need to be called many times but this not
  if (decl->var.scoped)
    return;
  decl->var.scoped = true;

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
  // string* realname = st_newc("$", st_enc_utf8);
  // st_append(&realname, decl->tpl.id->identifier.string);
  string* realname = decl->tpl.id->identifier.string;

  // add it!
  u64 ty_id = ts_type_size_s++;
  ts_type_table[ty_id].of = TY_TEMPLATE;
  ts_type_table[ty_id].id = realname;
  ts_type_table[ty_id].tpl.decl = decl;
  ts_type_table[ty_id].templated = true;

  array_new(&ts_type_table[ty_id].tpl.usedby);

  ast_t* attach_to = ast_get_scope(decl);
  log_silly("register template: %s", realname->value);
  hash_set(attach_to->block.types, realname->value, decl);

  return ty_id;
}

void ty_struct_add_virtual(ast_t* decl) {
  fl_assert(decl->type == AST_DECL_FUNCTION);

  string* id = decl->func.id->identifier.string;
  ast_t* params = decl->func.params;

  fl_assert(params->list.length == 1);
  u64 ty_id = params->list.values[0]->ty_id;
  ty_t* type = &ts_type_table[ty_id];

  if (type->of != TY_STRUCT) {
    ast_raise_error(
        decl, "function property first argument must be a struct given: %s",
        ty_to_string(ty_id)->value);
  }

  u64 max = type->structure.virtuals.length;
  for (u64 i = 0; i < max; ++i) {
    ast_t* ast = (ast_t*)type->structure.virtuals.values[i];
    fl_assert(ast != decl);
    if (st_cmp(ast->func.id->identifier.string, id) == 0) {
      ast_raise_error(decl, "function property redefinition (same name), "
                            "previously defined at %s",
                      ast_get_location(ast)->value);
    }
  }

  array_push(&type->structure.virtuals, decl);
}

ast_t* ty_get_virtual(u64 ty_id, string* id, bool look_up) {
  ty_t type = ty(ty_id);
  fl_assert(type.of == TY_STRUCT);
  u64 max = type.structure.virtuals.length;

  log_silly("search @ %lu", max);

  for (u64 i = 0; i < max; ++i) {
    ast_t* node = (ast_t*)type.structure.virtuals.values[i];
    log_silly("search: '%s'", node->func.id->identifier.string->value);

    if (st_cmp((const string*)node->func.id->identifier.string, id) == 0) {
      return node;
    }
  }
  // try to lookup -> implement here ?
  if (look_up && type.structure.from_tpl) {
    log_silly("look_up: type.structure.from_tpl %lu", type.structure.from_tpl);
    // REVIEW false has no meaning now, because tpl cannot inherit more than
    // once...
    // what happens in the future?
    return ty_get_virtual(type.structure.from_tpl, id, false);
  }

  return 0;
}
