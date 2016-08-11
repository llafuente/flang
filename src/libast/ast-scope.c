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

array* ast_scope_fns(ast_t* node, string* id) {
  array* arr = pool_new(sizeof(array));
  array* arr2;
  array_new(arr);

  char* cstr = id->value;
  ast_t* scope = node;
  ast_t* fn;
  do {
    scope = ast_get_scope(scope);

    fn = (ast_t*)hash_get(scope->block.types, cstr);
    if (fn && fn->type == AST_DECL_FUNCTION) {
      array_concat(arr, (array*)hash_get(scope->block.functions, cstr));
    }
  } while (scope->block.scope != AST_SCOPE_GLOBAL);

  if (arr->length) {
    return arr;
  }

  array_delete(arr);
  pool_free(arr);
  return 0;
}
