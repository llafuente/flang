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

void __ast_reduce_log(ast_t* node) {
  // this will mutate to expr-call
  // get each part code and inject it before in the list
  ast_t* list = node->log.list; // cache

  char buffer[512];
  buffer[0] = '\0';

  fl_assert(list->list.length == 1); // just one atm!

  // for (u64 i = 0; i < list->list.length; ++i) {
  {
    ast_t* arguments = ast_mk_list();
    ast_t* el = list->list.values[0];

    strcat(buffer, "$\x1B[36m(%s) %s\x1B[39m = ");
    strcat(buffer, ty_to_color(el->ty_id));
    char* b = buffer + strlen(buffer);
    ty_to_printf(el->ty_id, b);
    strcat(buffer, "\x1B[39m ");

    // arg 0: format
    buffer[strlen(buffer) - 1] = '\n';
    ast_mk_list_push(arguments, ast_mk_lit_string(buffer, false));

    // arg 1: type
    ast_mk_list_push(arguments,
                     ast_mk_lit_string2(ty_to_string(el->ty_id), false, true));

    // arg 2: code
    ast_mk_list_push(arguments,
                     ast_mk_lit_string2(ast_get_code(el), false, true));

    // arg 2+: logged values
    ty_t type = ty(el->ty_id);

    switch (type.of) {
    case TY_REFERENCE: {
      // auto-dereference
      ast_t* deref = ast_mk_lunary(el, '*');
      el->parent = deref;
      ast_mk_list_push(arguments, deref);
    } break;
    case TY_STRUCT: {
      // print each memeber
      array* props = (array*)&type.structure.properties;
      for (u64 j = 0; j < props->length; ++j) {
        ast_t* prop = ast_mk_lit_id(type.structure.properties.values[j], false);
        ast_t* member = ast_mk_member(el, prop, false, false);
        ast_mk_list_push(arguments, member);
      }
    } break;
    default:
      ast_mk_list_push(arguments, el);
    }

    ast_clear(node, AST_EXPR_CALL);
    node->call.callee = ast_mk_lit_id(st_newc("printf", st_enc_utf8), true);
    node->call.arguments = arguments;
  }

  // typesystem need to pass again...
  ast_parent(node);
}

ast_action_t __trav_reduced(ast_trav_mode_t mode, ast_t* node, ast_t* parent,
                            u64 level, void* userdata_in, void* userdata_out) {
  if (mode == AST_TRAV_LEAVE)
    return 0;
  switch (node->type) {
  case AST_STMT_LOG: {
    __ast_reduce_log(node);
    ++(*(u64*)userdata_out);
  } break;
  default: {} // avoid warning
  }

  return AST_SEARCH_CONTINUE;
}

// return error
ast_t* ast_reduce(ast_t* node) {
  u64 reduced;
  do {
    reduced = 0;
    ast_traverse(node, __trav_reduced, 0, 0, 0, (void*)&reduced);
    if (reduced) {
      _typesystem(node);
    }
  } while (reduced);

  return node;
}
