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

ast_action_t __trav_casting(ast_t* node, ast_t* parent, size_t level,
                            void* userdata_in, void* userdata_out) {
  switch (node->type) {
  case FL_AST_STMT_RETURN: {
    ts_cast_return(node);
  } break;
  case FL_AST_LIT_STRING: {
    node->ty_id = TS_STRING;
  } break;
  case FL_AST_LIT_IDENTIFIER: {
    if (node->identifier.resolve) {
      node->identifier.decl = ast_search_id_decl(node, node->identifier.string);
      if (!node->identifier.decl) {
        ast_raise_error(node, "Cannot find declaration");
      }

      if (node->parent->type != FL_AST_EXPR_CALL &&
          node->parent->type != FL_AST_EXPR_MEMBER) {
        // it's a var, copy type
        node->ty_id = ast_get_typeid(node->identifier.decl);
      }
    }

    if (node->parent->type == FL_AST_DTOR_VAR) {
      // if type is a function, in fact what we want is a
      // function pointer, so do it for easy to type
      if (ty_is_function(node->ty_id)) {
        node->ty_id = ty_create_wrapped(FL_POINTER, node->ty_id);
        node->parent->ty_id = node->ty_id;
        if (node->parent->var.type) {
          node->parent->var.type->ty_id = node->ty_id;
        }
      } else {
        node->parent->ty_id = node->ty_id;
      }
    }

  } break;
  case FL_AST_EXPR_MEMBER: {
    ts_cast_expr_member(node);
  } break;
  case FL_AST_EXPR_LUNARY: {
    ts_cast_lunary(node);
  } break;
  case FL_AST_EXPR_ASSIGNAMENT: {
    ts_cast_assignament(node);
  } break;
  case FL_AST_EXPR_CALL: {
    ts_cast_call(node);
  } break;
  case FL_AST_EXPR_BINOP: {
    ts_cast_binop(node);
  }
  default: {} // supress warning
  }
  return FL_AC_CONTINUE;
}

ast_action_t __ts_cast_operation_pass_cb(ast_t* node, ast_t* parent,
                                         size_t level, void* userdata_in,
                                         void* userdata_out) {
  if (node->type == FL_AST_CAST) {
    node->cast.operation = ts_cast_operation(node);
  }

  return FL_AC_CONTINUE;
}

ast_t* ts_pass(ast_t* node) {
  log_debug("(typesystem) ts_inference");
  ts_inference(node);
  if (ast_last_error_node != 0) {
    return node;
  }

  log_debug("(typesystem) casting");
  // first create casting
  ast_traverse(node, __trav_casting, 0, 0, 0, 0);
  if (ast_last_error_node != 0) {
    return node;
  }

  // validate casting, and assign a valid operation
  ast_traverse(node, __ts_cast_operation_pass_cb, 0, 0, 0, 0);
  if (ast_last_error_node != 0) {
    return node;
  }

  log_debug("(typesystem) done!");

  return node; // TODO this should be the error
}
