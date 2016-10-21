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
#include "flang/libast.h"
#include "flang/libts.h"
#include "flang/debug.h"

array* ast_get_scopes(ast_t* node) {
  array* ret = pool_new(sizeof(array));
  array_new(ret);

  do {
    node = ast_get_scope(node);

    array_push(ret, node);
    for (i64 i = 0; i < node->block.modules.length; ++i) {
      ast_t* module = (ast_t*)node->block.modules.values[i];
      fl_assert(module->program.body->type == AST_BLOCK);
      array_push(ret, module->program.body);
    }
  } while (node->block.scope != AST_SCOPE_GLOBAL);

  if (!ret->length) {
    array_delete(ret);
    pool_free(ret);
    return 0;
  }

  return ret;
}

ast_t* ast_scope_decl(ast_t* node, string* identifier) {
  ast_t* ret = 0;
  array* arr = 0;

  char* cstr = identifier->value;

  array* scopes = ast_get_scopes(node);
  if (!scopes)
    return 0;

  log_silly("searching in %lu scopes", scopes->length);

  ast_t* scope;
  for (u64 i = 0; i < scopes->length; ++i) {
    scope = (ast_t*)scopes->values[i];

    ret = (ast_t*)hash_get(scope->block.variables, cstr);
    if (ret) {
      return ret;
    }

    // TODO REVIEW why the first one only ? can remember why?!!
    arr = hash_get(scope->block.functions, cstr);
    if (arr) {
      return (ast_t*)array_get(arr, 0);
    }
  }

  pool_free(scopes);

  return 0;
}
// TODO REVIEW this is used?! this is wrong!
ast_t* ast_scope_var(ast_t* node, string* identifier) {
  ast_t* ret = 0;
  array* arr = 0;

  char* cstr = identifier->value;

  do {
    node = ast_get_scope(node);

    // found a variable!
    ret = (ast_t*)hash_get(node->block.variables, cstr);
    if (ret) {
      return ret;
    }
  } while (node->block.scope != AST_SCOPE_GLOBAL);

  return 0;
}

array* __target_arr = 0;
string* __target_id = 0;
void __foreach_function(char* key, void* ptr) {
  fl_assert(ptr != 0);

  array* list = (array*)ptr;
  for (u64 i = 0; i < list->length; ++i) {
    ast_t* node = (ast_t*)list->values[0];
    if (st_cmp(node->func.id->identifier.string, __target_id) == 0) {
      log_silly("function found!");
      array_push_unique(__target_arr, node);
    }
  }
}

array* ast_scope_fns(ast_t* node, string* id) {
  array* arr = __target_arr = pool_new(sizeof(array));
  __target_id = id;
  array_new(arr);

  array* scopes = ast_get_scopes(node);
  if (!scopes)
    return 0;

  log_silly("searching for '%s' in %lu scopes", id->value, scopes->length);

  for (u64 i = 0; i < scopes->length; ++i) {
    hash_each(((ast_t*)scopes->values[i])->block.functions, __foreach_function);
  }

  log_silly("found %lu", arr->length);
  if (arr->length) {
    return arr;
  }

  array_delete(arr);
  pool_free(arr);
  return 0;
}

ast_t* ast_scope_type(ast_t* node, string* id) {
  ast_t* ret_node;
  array* scopes = ast_get_scopes(node);
  if (!scopes)
    return 0;

  for (u64 i = 0; i < scopes->length; ++i) {
    ret_node = hash_get(((ast_t*)scopes->values[i])->block.types, id->value);
    if (ret_node != 0) {
      return ret_node;
    }
  }

  return 0;
}

/*
// NOTE as exposed @test-operator-overloading.fl
// maybe operator should be implemented for implicit reference

// this was in the old 'search for operators'
// apply some rules here about how we search for operators.
// operator[] modify struct, so first param must be a reference.
switch (operator) {
case TK_ACCESS_MOD:
case TK_ACCESS: {
  if (!ty_is_reference(ty_id)) {
    log_silly("search for a reference");
    ty_id = ty_create_wrapped(TY_REFERENCE, ty_id);
  }
} break;
default: {} // remove warning
}
*/
// TODO this should recieve left/right node because right node may need to be
// check for casting
ast_t* ast_scope_binop_operator(ast_t* node, int operator, u64 lty_id,
                                u64 rty_id) {
  u64 can_left_ty_id = ty_get_cannonical(lty_id);

  // first search
  array* scopes = ast_get_scopes(node);
  if (!scopes)
    return 0;

  log_silly("searching for '%d' in %lu scopes", operator, scopes->length);

  for (u64 i = 0; i < scopes->length; ++i) {
    hash_t* hash = ((ast_t*)scopes->values[i])->block.functions;
    HASH_EACH(hash, entry, {
      if (entry->value) {
        array* functions = (array*)entry->value;
        for (u64 j = 0; j < functions->length; ++j) {
          ast_t* fn = functions->values[j];
          if (fn->func.type == AST_FUNC_OPERATOR &&
              fn->func.operator== operator) {
            log_silly("operator found! check params!");
            // now check both types
            ast_t* params = fn->func.params;
            fl_assert(params->list.length == 2); // jsut to be sure ^^

            // TODO REVIEW right now we only check the first parameter
            // expecting the second to be castable, working but had
            // edge cases
            // params->list.values[1]->ty_id == rty_id
            if (params->list.values[0]->ty_id == lty_id ||
                params->list.values[0]->ty_id == can_left_ty_id) {
              log_silly("function found[id=%lu]", fn->id);
              return fn;
            }
          }
        }
      }
    });
  }

  // if not found, maybe need to implement a template
  // search in the operator list in the struct

  ast_t* fn = 0;
  lty_id = ty_get_cannonical(lty_id);
  ty_t type = ty(lty_id);

  log_silly("cannonical left is: %s", ty_to_string(lty_id)->value);
  log_silly("%d, %d", type.of == TY_STRUCT, type.structure.from_tpl);

  // first argument is a implemented struct
  if (type.of == TY_STRUCT && type.structure.from_tpl) {
    log_silly("maybe there is an operator in my father?");
    fn = ty_get_operator(type.structure.from_tpl, 0, operator, false);

    if (fn) { // implement!
      array* type_list = pool_new(sizeof(array));
      array_push(type_list, lty_id);
      array_push(type_list, rty_id);

      fn = ast_implement_fn2(type_list, fn, 0);
    }
  }

  return fn;
}
