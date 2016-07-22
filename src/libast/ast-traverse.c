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
#include "flang/libast.h"
#include "flang/libts.h"
#include "flang/debug.h"

ast_action_t __ast_traverse(ast_t* ast, ast_cb_t cb, ast_t* parent, u64 level,
                            void* userdata_in, void* userdata_out) {
#define TRAVERSE(node)                                                         \
  if (node) {                                                                  \
    switch (__ast_traverse(node, cb, ast, level, userdata_in, userdata_out)) { \
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
      for(u64 i = 0; i < (node)->list.length; ++i) {  \
      tmp = (ast_t*) (node->list.values)[i]; \
          switch (__ast_traverse(tmp, cb, ast, level, userdata_in,             \
                                 userdata_out)) {                              \
          case AST_SEARCH_SKIP:                                                \
          case AST_SEARCH_CONTINUE:                                            \
            continue;                                                          \
          case AST_SEARCH_STOP:                                                \
            return AST_SEARCH_STOP;                                            \
          }                                                                    \
      }                                                                        \
    }                                                                          \
  }

  if (!ast) {
    log_warning("ast_traverse: (nil)");
    return true; // is null but continue... its not an error
  }

  ++level;
  // stop if callback is false
  switch (cb(AST_TRAV_ENTER, ast, parent, level, userdata_in, userdata_out)) {
  case AST_SEARCH_STOP:
    return AST_SEARCH_STOP;
  case AST_SEARCH_SKIP:
    return AST_SEARCH_CONTINUE;
  case AST_SEARCH_CONTINUE: {
  }
  }

  switch (ast->type) {
  case AST_MODULE:
  case AST_PROGRAM:
    TRAVERSE(ast->program.body);
    break;
  case AST_BLOCK: {
    TRAVERSE(ast->block.body);
  } break;
  case AST_LIST: {
    TRAVERSE_LIST(ast);
  } break;
  case AST_EXPR_ASSIGNAMENT:
    TRAVERSE(ast->assignament.left);
    TRAVERSE(ast->assignament.right);
    break;
  case AST_EXPR_BINOP:
    TRAVERSE(ast->binop.left);
    TRAVERSE(ast->binop.right);
    break;
  case AST_EXPR_LUNARY:
    TRAVERSE(ast->lunary.element);
    break;
  case AST_EXPR_RUNARY:
    TRAVERSE(ast->runary.element);
    break;
  case AST_EXPR_CALL: {
    TRAVERSE(ast->call.callee);
    TRAVERSE(ast->call.arguments);
  } break;
  case AST_EXPR_MEMBER: {
    TRAVERSE(ast->member.left);
    TRAVERSE(ast->member.property);
  } break;
  case AST_DTOR_VAR: {
    TRAVERSE(ast->var.id);
    TRAVERSE(ast->var.type);
  } break;
  case AST_DECL_FUNCTION: {
    TRAVERSE(ast->func.attributes);
    TRAVERSE(ast->func.id);
    TRAVERSE(ast->func.ret_type);
    TRAVERSE(ast->func.params);
    TRAVERSE(ast->func.body);
  } break;
  case AST_PARAMETER: {
    TRAVERSE(ast->param.id);
    TRAVERSE(ast->param.type);
  } break;
  case AST_STMT_RETURN: {
    TRAVERSE(ast->ret.argument);
  } break;
  case AST_STMT_IF: {
    TRAVERSE(ast->if_stmt.test);
    TRAVERSE(ast->if_stmt.block);
    TRAVERSE(ast->if_stmt.alternate);
  } break;
  case AST_STMT_LOOP: {
    TRAVERSE(ast->loop.init);
    TRAVERSE(ast->loop.pre_cond);
    TRAVERSE(ast->loop.update);
    TRAVERSE(ast->loop.block);
    TRAVERSE(ast->loop.post_cond);
  } break;
  case AST_CAST: {
    TRAVERSE(ast->cast.type);
    TRAVERSE(ast->cast.element);
  } break;
  case AST_DECL_STRUCT: {
    TRAVERSE(ast->structure.id);
    TRAVERSE(ast->structure.fields);
    TRAVERSE(ast->structure.tpls);
  } break;
  case AST_DECL_STRUCT_FIELD: {
    TRAVERSE(ast->field.id);
    TRAVERSE(ast->field.type);
  } break;
  case AST_DECL_TEMPLATE: {
    TRAVERSE(ast->tpl.id);
  } break;
  case AST_EXPR_SIZEOF: {
    TRAVERSE(ast->sof.type);
  } break;
  case AST_TYPE: {
    TRAVERSE(ast->ty.id);
    TRAVERSE(ast->ty.children);
  } break;
  case AST_STMT_LOG: {
    TRAVERSE(ast->log.list);
  } break;
  case AST_ATTRIBUTE: {
    TRAVERSE(ast->attr.id);
    TRAVERSE(ast->attr.value);
  } break;
  case AST_IMPLEMENT: {
    TRAVERSE(ast->impl.uid);
    TRAVERSE(ast->impl.type_id);
    TRAVERSE(ast->impl.type_list);
  }
  default: {}
  }

  cb(AST_TRAV_LEAVE, ast, parent, level, userdata_in, userdata_out);

  return AST_SEARCH_CONTINUE;
}

void ast_traverse(ast_t* ast, ast_cb_t cb, ast_t* parent, u64 level,
                  void* userdata_in, void* userdata_out) {
  __ast_traverse(ast, cb, parent, level, userdata_in, userdata_out);
}

void ast_traverse_list(ast_t* node, ast_cb_t cb, ast_t* until, u64 level,
                       void* userdata_in, void* userdata_out) {
  assert(node->type == AST_LIST);

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
