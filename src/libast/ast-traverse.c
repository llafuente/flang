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

ast_action_t __ast_traverse(ast_t* node, ast_cb_t cb, ast_t* parent, u64 level,
                            void* userdata_in, void* userdata_out) {
#define TRAVERSE(target)                                                       \
  if (node->target) {                                                          \
    switch (__ast_traverse(node->target, cb, node, level, userdata_in,         \
                           userdata_out)) {                                    \
    case AST_SEARCH_STOP:                                                      \
      return AST_SEARCH_STOP;                                                  \
    case AST_SEARCH_SKIP:                                                      \
      return AST_SEARCH_CONTINUE;                                              \
    case AST_SEARCH_CONTINUE: {                                                \
    }                                                                          \
    }                                                                          \
  }

#define TRAVERSE_LIST(node)                                                    \
  {                                                                            \
    u64 i = 0;                                                                 \
    ast_t* tmp;                                                                \
                                                                               \
    if (node) {                                                                \
      for (u64 i = 0; i < (node)->list.length; ++i) {                          \
        tmp = (ast_t*)(node->list.values)[i];                                  \
        switch (                                                               \
            __ast_traverse(tmp, cb, node, level, userdata_in, userdata_out)) { \
        case AST_SEARCH_SKIP:                                                  \
        case AST_SEARCH_CONTINUE:                                              \
          continue;                                                            \
        case AST_SEARCH_STOP:                                                  \
          return AST_SEARCH_STOP;                                              \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

  if (!node) {
    log_warning("ast_traverse: (nil)");
    return true; // is null but continue... its not an error
  }

  ++level;
  // stop if callback is false
  switch (cb(AST_TRAV_ENTER, node, parent, level, userdata_in, userdata_out)) {
  case AST_SEARCH_STOP:
    return AST_SEARCH_STOP;
  case AST_SEARCH_SKIP:
    return AST_SEARCH_CONTINUE;
  case AST_SEARCH_CONTINUE: {
  }
  }

  switch (node->type) {
  case AST_MODULE:
  case AST_PROGRAM:
    TRAVERSE(program.body);
    break;
  case AST_BLOCK: {
    TRAVERSE(block.body);
    for (u64 i = 0; i < node->block.modules.length; ++i) {
      TRAVERSE(block.modules.values[i]);
    }
  } break;
  case AST_LIST: {
    TRAVERSE_LIST(node);
  } break;
  case AST_EXPR_ASSIGNAMENT:
    TRAVERSE(assignament.left);
    TRAVERSE(assignament.right);
    break;
  case AST_EXPR_BINOP:
    TRAVERSE(binop.left);
    TRAVERSE(binop.right);
    break;
  case AST_EXPR_LUNARY:
    TRAVERSE(lunary.element);
    break;
  case AST_EXPR_RUNARY:
    TRAVERSE(runary.element);
    break;
  case AST_EXPR_CALL: {
    TRAVERSE(call.callee);
    TRAVERSE(call.arguments);
  } break;
  case AST_EXPR_MEMBER: {
    TRAVERSE(member.left);
    TRAVERSE(member.property);
  } break;
  case AST_DTOR_VAR: {
    TRAVERSE(var.id);
    TRAVERSE(var.type);
  } break;
  case AST_DECL_FUNCTION: {
    TRAVERSE(func.attributes);
    TRAVERSE(func.id);
    TRAVERSE(func.ret_type);
    TRAVERSE(func.params);
    TRAVERSE(func.body);
  } break;
  case AST_PARAMETER: {
    TRAVERSE(param.id);
    TRAVERSE(param.type);
  } break;
  case AST_STMT_RETURN: {
    TRAVERSE(ret.argument);
  } break;
  case AST_STMT_IF: {
    TRAVERSE(if_stmt.test);
    TRAVERSE(if_stmt.block);
    TRAVERSE(if_stmt.alternate);
  } break;
  case AST_STMT_LOOP: {
    TRAVERSE(loop.init);
    TRAVERSE(loop.pre_cond);
    TRAVERSE(loop.update);
    TRAVERSE(loop.block);
    TRAVERSE(loop.post_cond);
  } break;
  case AST_CAST: {
    TRAVERSE(cast.type);
    TRAVERSE(cast.element);
  } break;
  case AST_DECL_STRUCT: {
    TRAVERSE(structure.id);
    TRAVERSE(structure.fields);
    TRAVERSE(structure.tpls);
  } break;
  case AST_DECL_STRUCT_FIELD: {
    TRAVERSE(field.id);
    TRAVERSE(field.type);
  } break;
  case AST_DECL_STRUCT_ALIAS: {
    TRAVERSE(alias.name);
    TRAVERSE(alias.id);
  }
  case AST_DECL_TEMPLATE: {
    TRAVERSE(tpl.id);
  } break;
  case AST_EXPR_SIZEOF: {
    TRAVERSE(sof.type);
  } break;
  case AST_EXPR_TYPEOF: {
    TRAVERSE(tof.expr);
  } break;
  case AST_TYPE: {
    TRAVERSE(ty.id);
    TRAVERSE(ty.children);
  } break;
  case AST_STMT_LOG: {
    TRAVERSE(log.list);
  } break;
  case AST_ATTRIBUTE: {
    TRAVERSE(attr.id);
    TRAVERSE(attr.value);
  } break;
  case AST_IMPLEMENT: {
    TRAVERSE(impl.uid);
    TRAVERSE(impl.type_id);
    TRAVERSE(impl.type_list);
  } break;
  case AST_NEW: {
    TRAVERSE(delete.expr);
  } break;
  case AST_DELETE: {
    TRAVERSE(delete.expr);
  } break;
  default: {}
  }

  cb(AST_TRAV_LEAVE, node, parent, level, userdata_in, userdata_out);

  return AST_SEARCH_CONTINUE;
}

void ast_traverse(ast_t* ast, ast_cb_t cb, ast_t* parent, u64 level,
                  void* userdata_in, void* userdata_out) {
  __ast_traverse(ast, cb, parent, level, userdata_in, userdata_out);
}

void ast_traverse_list(ast_t* node, ast_cb_t cb, ast_t* until, u64 level,
                       void* userdata_in, void* userdata_out) {
  fl_assert(node->type == AST_LIST);

  u64 i;
  for (i = 0; i < node->list.length; ++i) {
    // exit when reach parent
    if (node->list.values[i] == until) {
      return;
    }

    switch (__ast_traverse(node->list.values[i], cb, node, level + 1,
                           userdata_in, userdata_out)) {
    case AST_SEARCH_SKIP:
    case AST_SEARCH_CONTINUE:
      continue;
    case AST_SEARCH_STOP:
      return;
    }
  }
}
