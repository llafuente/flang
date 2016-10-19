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
#include "flang/libast.h"
#include "flang/debug.h"

void ts_check_operator_overloading(ast_t* node) {
  // operator[] recieve always references
  // fl_assert(false);
  switch (node->func.operator) {
  case TK_ACCESS_MOD:
  case TK_ACCESS: {
    if (node->func.params->list.length != 2) {
      ast_raise_error(
          node, "type error, operator[] requires two parameters given: %lu",
          node->func.params->list.length);
    }
    ast_t* p0 = node->func.params->list.values[0];
    if (!ty_is_reference(p0->ty_id)) {
      ast_raise_error(node, "type error, operator[] requires first parameter "
                            "to be a reference given (%s)",
                      ty_to_string(p0->ty_id)->value);
    }

    // must return a reference to something
    if (node->func.operator== TK_ACCESS_MOD) {
      if (!ty_is_reference(node->func.ret_type->ty_id)) {
        ast_raise_error(node, "type error, operator[]= requires to return a "
                              "reference, returned (%s)",
                        ty_to_string(node->func.ret_type->ty_id)->value);
      }
    }
  } break;
  default: {} // remove warning
  }
}

ast_action_t ts_check_no_assignament_cb(AST_CB_T_HEADER) {
  if (node->type == AST_EXPR_ASSIGNAMENT) {
    ast_raise_error(node,
                    "syntax error, assignament expression is fobidden here.");
  }

  return AST_SEARCH_CONTINUE;
}

void ts_check_no_assignament(ast_t* node) {
  ast_traverse(node, ts_check_no_assignament_cb, 0, 0, 0, 0);
}
