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

void* ast_reverse(ast_t* ast, fl_ast_ret_cb_t cb, ast_t* parent, size_t level,
                  void* userdata) {
  void* ret;
#define REVERSE(node)                                                          \
  if (node) {                                                                  \
    ret = ast_reverse(node, cb, ast, level, userdata);                         \
    if (ret)                                                                   \
      return ret;                                                              \
  }

#define REVERSE_LIST(node)                                                     \
  {                                                                            \
    size_t i = 0;                                                              \
    ast_t* tmp;                                                                \
                                                                               \
    if (node) {                                                                \
      while ((tmp = node[i++]) != 0) {                                         \
        /* do not reverse list, just call cb*/                                 \
        if (!cb(tmp, parent, level, userdata, &ret)) {                         \
          return ret;                                                          \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

  if (!ast) {
    log_warning("ast_reverse: (nil)");
    return 0;
  }

  ++level;
  // stop if callback is false
  if (!cb(ast, parent, level, userdata, &ret)) {
    return ret;
  }

  switch (ast->type) {
  case FL_AST_MODULE:
  case FL_AST_PROGRAM:
    if (ast->program.core) {
      log_verbose("reverse core -> traverse");
      ast_traverse(ast->program.core, cb, 0, 0, userdata);
    }
    break;
  case FL_AST_BLOCK: {
    REVERSE_LIST(ast->block.body);
  } break;
  case FL_AST_LIST: {
    REVERSE_LIST(ast->list.elements);
    if (ast->parent->type == FL_AST_DECL_FUNCTION ||
        ast->parent->type == FL_AST_EXPR_CALL) {
      // recursion!
      return 0;
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
    REVERSE(ast->call.arguments);
  } break;
  case FL_AST_DTOR_VAR:
    break;
  case FL_AST_DECL_FUNCTION: {
    REVERSE(ast->func.params);
  } break;
  case FL_AST_PARAMETER:
    break;
  case FL_AST_STMT_RETURN: {
  }
  case FL_AST_CAST: {
  }
  default: {}
  }

  REVERSE(ast->parent);
  return 0;
}
