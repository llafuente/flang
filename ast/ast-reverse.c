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
#include "flang/ast.h"
#include "flang/libast.h"
#include "flang/debug.h"

bool __ast_reverse(ast_t* node, ast_cb_t cb, ast_t* parent, u64 level,
                   void* userdata_in, void* userdata_out) {

#define REVERSE(child)                                                         \
  if (child) {                                                                 \
    if (!__ast_reverse(child, cb, node, level, userdata_in, userdata_out)) {   \
      return false;                                                            \
    }                                                                          \
  }

  if (!node) {
    log_warning("ast_reverse: (nil)");
    return true;
  }

  ++level;
  // stop if callback is false
  if (!cb(node, parent, level, userdata_in, userdata_out)) {
    return false;
  }

  switch (node->type) {
  case FL_AST_MODULE:
    ast_traverse(node, cb, parent, 0, userdata_in, userdata_out);
    break;
  case FL_AST_PROGRAM:
    if (node->parent) { // TODO module should have PROGRAM inside?
      ast_traverse(node->program.body, cb, node, 0, userdata_in, userdata_out);
    }
    break;
  case FL_AST_BLOCK: {
    if (node->parent->type == FL_AST_PROGRAM && node->parent->parent) {
      // do not traverse current program
      ast_traverse(node, cb, parent, 0, userdata_in, userdata_out);
    }
  } break;
  case FL_AST_LIST: {
    if (parent) { // do not reverse current-first node
      ast_traverse_list(node, cb, parent, 0, userdata_in, userdata_out);
    }
  }; break;
  case FL_AST_EXPR_ASSIGNAMENT:
    break;
  case FL_AST_EXPR_BINOP:
    break;
  case FL_AST_EXPR_LUNARY:
    break;
  case FL_AST_EXPR_RUNARY:
    break;
  case FL_AST_EXPR_CALL: {
    if (parent) {
      ast_traverse(node->call.arguments, cb, parent, 0, userdata_in,
                   userdata_out);
    }
  } break;
  case FL_AST_DTOR_VAR:
    break;
  case FL_AST_DECL_FUNCTION: {
    if (node->func.params != parent) {
      ast_traverse(node->func.params, cb, parent, 0, userdata_in, userdata_out);
    }
  } break;
  case FL_AST_PARAMETER:
    break;
  case FL_AST_STMT_RETURN: {
  }
  case FL_AST_CAST: {
  }
  default: {}
  }

  REVERSE(node->parent);
  return true;
}

void ast_reverse(ast_t* node, ast_cb_t cb, ast_t* parent, u64 level,
                 void* userdata_in, void* userdata_out) {
  __ast_reverse(node, cb, parent, level, userdata_in, userdata_out);
}
