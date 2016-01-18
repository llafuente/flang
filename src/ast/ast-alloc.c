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

ast_t* ast_new() {
  return (ast_t*)pool_new(sizeof(ast_t));
  // return (ast_t*)calloc(1, sizeof(ast_t));
}

void __ast_delete_list(ast_t** list) {
  size_t i = 0;
  ast_t* tmp;

  if (list) {
    while ((tmp = list[i++]) != 0) {
      ast_delete(tmp);
    }
  }

  // pool is used to handle memory on errors
  // free(list);
}

void __ast_delete_props(ast_t* ast) {

#define SAFE_DEL(test)                                                         \
  if (test) {                                                                  \
    ast_delete(test);                                                          \
    test = 0;                                                                  \
  }

#define SAFE_DEL_LIST(test)                                                    \
  if (test) {                                                                  \
    __ast_delete_list(test);                                                   \
    test = 0;                                                                  \
  }

#define SAFE_DEL_STR(test)                                                     \
  if (test) {                                                                  \
    st_delete(&test);                                                          \
  }

  switch (ast->type) {
  case FL_AST_MODULE:
  case FL_AST_PROGRAM: {
    SAFE_DEL(ast->program.body);
    // TODO remove! tk_tokens_delete(ast->program.tokens);
    SAFE_DEL_STR(ast->program.code);
    SAFE_DEL_STR(ast->program.path);
    if (ast->program.file) {
      free(ast->program.file);
    }
  } break;
  case FL_AST_BLOCK: {
    SAFE_DEL(ast->block.body);
    // hash_delete(ast->block.types);
    // hash_delete(ast->block.functions);
    // hash_delete(ast->block.variables);
  } break;
  case FL_AST_LIST: {
    SAFE_DEL_LIST(ast->list.elements);
  } break;
  case FL_AST_EXPR_ASSIGNAMENT: {
    SAFE_DEL(ast->assignament.left);
    SAFE_DEL(ast->assignament.right);
  } break;
  case FL_AST_EXPR_BINOP: {
    SAFE_DEL(ast->binop.left);
    SAFE_DEL(ast->binop.right);
  } break;
  case FL_AST_EXPR_LUNARY: {
    SAFE_DEL(ast->lunary.element);
  } break;
  case FL_AST_EXPR_RUNARY: {
    SAFE_DEL(ast->runary.element);
  } break;
  case FL_AST_EXPR_CALL: {
    SAFE_DEL(ast->call.callee);
    SAFE_DEL(ast->call.arguments);
  } break;
  case FL_AST_EXPR_MEMBER: {
    SAFE_DEL(ast->member.left);
    SAFE_DEL(ast->member.property);
  } break;
  case FL_AST_LIT_STRING:
    SAFE_DEL_STR(ast->string.value);
    break;
  case FL_AST_LIT_IDENTIFIER:
    SAFE_DEL_STR(ast->identifier.string);
    break;
  case FL_AST_DTOR_VAR:
    SAFE_DEL(ast->var.id);
    SAFE_DEL(ast->var.type);
    break;
  case FL_AST_DECL_FUNCTION:
    SAFE_DEL(ast->func.id);
    SAFE_DEL_STR(ast->func.uid);
    SAFE_DEL(ast->func.ret_type);
    SAFE_DEL(ast->func.params);
    if (ast->func.body) {
      ast_delete(ast->func.body);
      ast->func.body = 0;
    }
    free(ast->func.body);
    break;
  case FL_AST_PARAMETER: {
    SAFE_DEL(ast->param.id);
    SAFE_DEL(ast->param.type);
  } break;
  case FL_AST_STMT_RETURN: {
    SAFE_DEL(ast->ret.argument);
  } break;
  case FL_AST_STMT_IF: {
    SAFE_DEL(ast->if_stmt.test);
    SAFE_DEL(ast->if_stmt.block);
    SAFE_DEL(ast->if_stmt.alternate);
  }
  case FL_AST_STMT_LOOP: {
    SAFE_DEL(ast->loop.init);
    SAFE_DEL(ast->loop.pre_cond);
    SAFE_DEL(ast->loop.update);
    SAFE_DEL(ast->loop.block);
    SAFE_DEL(ast->loop.post_cond);
  }
  case FL_AST_CAST: {
    SAFE_DEL(ast->cast.element);
  } break;
  case FL_AST_DECL_STRUCT: {
    SAFE_DEL(ast->structure.id);
    SAFE_DEL(ast->structure.fields);
  } break;
  case FL_AST_DECL_STRUCT_FIELD: {
    SAFE_DEL(ast->field.id);
    SAFE_DEL(ast->field.type);
  } break;
  case FL_AST_STMT_COMMENT: {
    SAFE_DEL_STR(ast->comment.text);
  } break;
  case FL_AST_TYPE: {
    SAFE_DEL(ast->ty.id);
    SAFE_DEL(ast->ty.child);
  }
  case FL_AST_ERROR: {
    SAFE_DEL_STR(ast->err.message);
    SAFE_DEL_STR(ast->err.type);
  }
  default: {}
  }
}

void ast_delete(ast_t* node) {
  __ast_delete_props(node);
  // pool is used to handle memory on errors
  // free(ast);
}

ast_t* __ast_clone(ast_t* node) {
  ast_t* t = ast_new();
  memcpy(t, node, sizeof(ast_t));

  // TODO this may even not be necessary
  switch (t->type) {
  case FL_AST_LIT_IDENTIFIER:
    t->identifier.string = st_clone(node->identifier.string);
    break;
  default: {} // supress warning
  }

#define CLONE(target)                                                          \
  if (node->target) {                                                          \
    t->target = __ast_clone(node->target);                                     \
  }

  // traverse
  switch (node->type) {
  case FL_AST_MODULE:
  case FL_AST_PROGRAM:
    CLONE(program.body);
    break;
  case FL_AST_BLOCK: {
    CLONE(block.body);
  } break;
  case FL_AST_LIST: {
    size_t i = 0;
    size_t max = node->list.count;
    ast_t* tmp;

    // +1 null at end
    t->list.elements = pool_new(max * sizeof(ast_t*) + 1);

    for (i = 0; i < max; ++i) {
      t->list.elements[i] = __ast_clone(node->list.elements[i]);
    }

  } break;
  case FL_AST_EXPR_ASSIGNAMENT:
    CLONE(assignament.left);
    CLONE(assignament.right);
    break;
  case FL_AST_EXPR_BINOP:
    CLONE(binop.left);
    CLONE(binop.right);
    break;
  case FL_AST_EXPR_LUNARY:
    CLONE(lunary.element);
    break;
  case FL_AST_EXPR_RUNARY:
    CLONE(runary.element);
    break;
  case FL_AST_EXPR_CALL: {
    CLONE(call.callee);
    CLONE(call.arguments);
  } break;
  case FL_AST_EXPR_MEMBER: {
    CLONE(member.left);
    CLONE(member.property);
  } break;
  case FL_AST_DTOR_VAR: {
    CLONE(var.id);
    CLONE(var.type);
  } break;
  case FL_AST_DECL_FUNCTION: {
    CLONE(func.attributes);
    CLONE(func.id);
    CLONE(func.ret_type);
    CLONE(func.params);
    CLONE(func.body);
  } break;
  case FL_AST_PARAMETER: {
    CLONE(param.id);
    CLONE(param.type);
  } break;
  case FL_AST_STMT_RETURN: {
    CLONE(ret.argument);
  } break;
  case FL_AST_STMT_IF: {
    CLONE(if_stmt.test);
    CLONE(if_stmt.block);
    CLONE(if_stmt.alternate);
  } break;
  case FL_AST_STMT_LOOP: {
    CLONE(loop.init);
    CLONE(loop.pre_cond);
    CLONE(loop.update);
    CLONE(loop.block);
    CLONE(loop.post_cond);
  } break;
  case FL_AST_CAST: {
    CLONE(cast.type);
    CLONE(cast.element);
  } break;
  case FL_AST_DECL_STRUCT: {
    CLONE(structure.id);
    CLONE(structure.fields);
  } break;
  case FL_AST_DECL_STRUCT_FIELD: {
    CLONE(field.id);
    CLONE(field.type);
  } break;
  case FL_AST_DECL_TEMPLATE: {
    CLONE(tpl.id);
    CLONE(tpl.block);
  } break;
  case FL_AST_EXPR_SIZEOF: {
    CLONE(sof.type);
  } break;
  case FL_AST_TYPE: {
    CLONE(ty.id);
    CLONE(ty.child);
  } break;
  case FL_AST_STMT_LOG: {
    CLONE(log.list);
  } break;
  case FL_AST_ATTRIBUTE: {
    CLONE(attr.id);
    CLONE(attr.value);
  } break;
  default: {}
  }
  return t;
}

ast_t* ast_clone(ast_t* node) {
  ast_t* ret = __ast_clone(node);
  ast_parent(ret);
  return ret;
}
