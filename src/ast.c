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

void fl_ast_traverse(fl_ast_t* ast, fl_ast_cb_t cb, fl_ast_t* parent,
                     size_t level) {
  if (!ast) {
    printf("(null)\n");
    return;
  }

  ++level;
  cb(ast, parent, level);

  switch (ast->type) {
  case FL_AST_PROGRAM:
    fl_ast_traverse(ast->program.body, cb, ast, level);
    break;
  case FL_AST_BLOCK: {
    size_t i = 0;
    fl_ast_t* tmp;

    if (ast->block.body) {
      while ((tmp = ast->block.body[i++]) != 0) {
        fl_ast_traverse(tmp, cb, ast, level);
      }
    }
  } break;
  case FL_AST_EXPR_ASSIGNAMENT:
    fl_ast_traverse(ast->assignament.left, cb, ast, level);
    fl_ast_traverse(ast->assignament.right, cb, ast, level);
    break;
  case FL_AST_EXPR_BINOP:
    fl_ast_traverse(ast->binop.left, cb, ast, level);
    fl_ast_traverse(ast->binop.right, cb, ast, level);
    break;
  case FL_AST_EXPR_LUNARY:
    fl_ast_traverse(ast->lunary.element, cb, ast, level);
    break;
  case FL_AST_EXPR_RUNARY:
    fl_ast_traverse(ast->runary.element, cb, ast, level);
    break;
  case FL_AST_DTOR_VAR:
    fl_ast_traverse(ast->var.id, cb, ast, level);
    fl_ast_traverse(ast->var.type, cb, ast, level);
    break;
  case FL_AST_DECL_FUNCTION: {
    fl_ast_traverse(ast->func.id, cb, ast, level);
    // fl_ast_traverse(ast->var.type, cb, ast, level);

    if (ast->func.params) {
      size_t i = 0;
      fl_ast_t* tmp;

      while ((tmp = ast->func.params[i++]) != 0) {
        fl_ast_traverse(tmp, cb, ast, level);
      }
    }
    if (ast->func.body) {
      fl_ast_traverse(ast->func.body, cb, ast, level);
    }
  } break;
  default: {}
  }
}
void fl_ast_parent_cb(fl_ast_t* node, fl_ast_t* parent, size_t level) {
  node->parent = parent;
}

void fl_ast_parent(fl_ast_t* root) {
  fl_ast_traverse(root, fl_ast_parent_cb, 0, 0);
}

void fl_ast_delete_list(fl_ast_t** list) {
  size_t i = 0;
  fl_ast_t* tmp;

  if (list) {
    while ((tmp = list[i++]) != 0) {
      fl_ast_delete(tmp);
    }
  }

  free(list);
}
void fl_ast_delete(fl_ast_t* ast) {
  // fprintf(stderr, "ast [%p]", ast);

  switch (ast->type) {
  case FL_AST_PROGRAM:
    if (ast->program.body) {
      fl_ast_delete(ast->program.body);
    }
    fl_tokens_delete(ast->program.tokens);
    break;
  case FL_AST_BLOCK: {
    fl_ast_delete_list(ast->block.body);
    ast->block.body = 0;
  } break;
  case FL_AST_EXPR_ASSIGNAMENT:
    if (ast->assignament.left) {
      fl_ast_delete(ast->assignament.left);
      ast->assignament.left = 0;
    }

    if (ast->assignament.right) {
      fl_ast_delete(ast->assignament.right);
      ast->assignament.right = 0;
    }
    break;
  case FL_AST_EXPR_BINOP:
    if (ast->binop.left) {
      fl_ast_delete(ast->binop.left);
      ast->binop.left = 0;
    }
    if (ast->binop.right) {
      fl_ast_delete(ast->binop.right);
      ast->binop.right = 0;
    }
    break;
  case FL_AST_EXPR_LUNARY:
    if (ast->lunary.element) {
      fl_ast_delete(ast->lunary.element);
      ast->lunary.element = 0;
    }
    break;
  case FL_AST_EXPR_RUNARY:
    if (ast->runary.element) {
      fl_ast_delete(ast->runary.element);
      ast->runary.element = 0;
    }
    break;
  case FL_AST_LIT_IDENTIFIER:
    st_delete(&ast->identifier.string);
    break;
  case FL_AST_DTOR_VAR:
    if (ast->var.id) {
      fl_ast_delete(ast->var.id);
    }
    if (ast->var.type) {
      fl_ast_delete(ast->var.type);
    }
    break;
  case FL_AST_DECL_FUNCTION:
    if (ast->func.id) {
      fl_ast_delete(ast->func.id);
    }
    if (ast->func.params) {
      size_t i = 0;
      fl_ast_t* tmp;

      while ((tmp = ast->func.params[i++]) != 0) {
        fl_ast_delete(tmp);
      }

      free(ast->func.params);
      ast->func.params = 0;
    }
    if (ast->func.body) {
      fl_ast_delete(ast->func.body);
      ast->func.body = 0;
    }
    free(ast->func.body);
    break;
  default: {}
  }
  free(ast);
}

void fl_ast_debug_cb(fl_ast_t* node, fl_ast_t* parent, size_t level) {
  if (!node) {
    printf("(null)\n");
    return;
  }
  level = level * 2;
  switch (node->type) {
  case FL_AST_PROGRAM:
    printf("%*s- program [%p]\n", (int)level, " ", node);
    break;
  case FL_AST_BLOCK:
    printf("%*s- block [%p]\n", (int)level, " ", node);
    break;
  case FL_AST_EXPR_ASSIGNAMENT:
    printf("%*s- assignament [%p]\n", (int)level, " ", node);
    break;
  case FL_AST_EXPR_BINOP:
    printf("%*s- binop (%d) [%p]\n", (int)level, " ", node->binop.operator,
           node);
    break;
  case FL_AST_LIT_NUMERIC:
    printf("%*s- number [%p]\n", (int)level, " ", node);
    break;
  case FL_AST_LIT_IDENTIFIER:
    printf("%*s- identifier (%s) [%p]\n", (int)level, " ",
           node->identifier.string->value, node);
    break;
  case FL_AST_EXPR_LUNARY:
    printf("%*s- lunary (%d) [%p]\n", (int)level, " ", node->lunary.operator,
           node);
    break;
  case FL_AST_EXPR_RUNARY:
    printf("%*s- runary (%d) [%p]\n", (int)level, " ", node->runary.operator,
           node);
    break;
  case FL_AST_DTOR_VAR:
    printf("%*s- variable [%p]\n", (int)level, " ", node);
    break;
  case FL_AST_TYPE:
    printf("%*s- type (%d) [%p]\n", (int)level, " ", node->idtype.of, node);
    break;
  case FL_AST_DECL_FUNCTION:
    printf("%*s- function [%p]\n", (int)level, " ", node);
    break;
  case FL_AST_ERROR:
    printf("%*s- ERROR %s [%p]\n", (int)level, " ", node->err.str, node);
    break;
  default: {}
  }

  printf("%*s[%3zu:%3zu - %3zu:%3zu]\n", (int)level, " ",
         node->token_start->start.column, node->token_start->start.line,
         node->token_end->end.column, node->token_end->end.line);
}

fl_ast_t* fl_ast_search_decl_var(fl_ast_t* node, string* name) {
  while ((node = node->parent) != 0) {
    if (node->type == FL_AST_BLOCK) {
      // search in the list
      size_t i = 0;
      fl_ast_t* tmp;

      if (node->block.body) {
        while ((tmp = node->block.body[i++]) != 0) {
          if (tmp->type == FL_AST_DTOR_VAR &&
              st_cmp(name, tmp->var.id->identifier.string) == 0) {
            return tmp;
          }
        }
      }
    }
  }

  return 0;
}
