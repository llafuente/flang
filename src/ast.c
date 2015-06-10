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
                     size_t level, void* userdata) {
#define TRAVERSE(node)                                                         \
  if (node) {                                                                  \
    fl_ast_traverse(node, cb, ast, level, userdata);                           \
  }

#define TRAVERSE_LIST(node)                                                    \
  {                                                                            \
    size_t i = 0;                                                              \
    fl_ast_t* tmp;                                                             \
                                                                               \
    if (node) {                                                                \
      while ((tmp = node[i++]) != 0) {                                         \
        TRAVERSE(tmp);                                                         \
      }                                                                        \
    }                                                                          \
  }

  if (!ast) {
    printf("(fl_ast_traverse) : null\n");
    return;
  }

  ++level;
  // stop if callback is false
  if (!cb(ast, parent, level, userdata)) {
    return;
  }

  switch (ast->type) {
  case FL_AST_PROGRAM:
    TRAVERSE(ast->program.body);
    break;
  case FL_AST_BLOCK: {
    TRAVERSE_LIST(ast->block.body);
  } break;
  case FL_AST_EXPR_ASSIGNAMENT:
    TRAVERSE(ast->assignament.left);
    TRAVERSE(ast->assignament.right);
    break;
  case FL_AST_EXPR_BINOP:
    TRAVERSE(ast->binop.left);
    TRAVERSE(ast->binop.right);
    break;
  case FL_AST_EXPR_LUNARY:
    TRAVERSE(ast->lunary.element);
    break;
  case FL_AST_EXPR_RUNARY:
    TRAVERSE(ast->runary.element);
    break;
  case FL_AST_EXPR_CALL: {
    TRAVERSE(ast->call.callee);
    TRAVERSE_LIST(ast->call.arguments);
  } break;
  case FL_AST_DTOR_VAR:
    TRAVERSE(ast->var.id);
    TRAVERSE(ast->var.type);
    break;
  case FL_AST_DECL_FUNCTION: {
    TRAVERSE(ast->func.id);
    TRAVERSE(ast->func.ret_type);
    TRAVERSE_LIST(ast->func.params);
    TRAVERSE(ast->func.body);
  } break;
  case FL_AST_PARAMETER:
    TRAVERSE(ast->param.id);
    TRAVERSE(ast->param.type);
    break;
  case FL_AST_STMT_RETURN: {
    TRAVERSE(ast->ret.argument);
  }
  case FL_AST_CAST: {
    TRAVERSE(ast->cast.to);
    TRAVERSE(ast->cast.right);
  }
  default: {}
  }
}

void fl_ast_reverse(fl_ast_t* ast, fl_ast_cb_t cb, fl_ast_t* parent,
                    size_t level, void* userdata) {
#define REVERSE(node)                                                          \
  if (node) {                                                                  \
    fl_ast_reverse(node, cb, ast, level, userdata);                            \
  }

#define REVERSE_LIST(node)                                                     \
  {                                                                            \
    size_t i = 0;                                                              \
    fl_ast_t* tmp;                                                             \
                                                                               \
    if (node) {                                                                \
      while ((tmp = node[i++]) != 0) {                                         \
        /* do not reverse list, just call cb*/                                 \
        if (!cb(ast, parent, level, userdata)) {                               \
          return;                                                              \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

  if (!ast) {
    printf("(fl_ast_reverse) : null\n");
    return;
  }

  ++level;
  // stop if callback is false
  if (!cb(ast, parent, level, userdata)) {
    return;
  }

  switch (ast->type) {
  case FL_AST_PROGRAM:
    break;
  case FL_AST_BLOCK: {
    REVERSE_LIST(ast->block.body);
  } break;
  case FL_AST_EXPR_ASSIGNAMENT:
    break;
  case FL_AST_EXPR_BINOP:
    break;
  case FL_AST_EXPR_LUNARY:
    break;
  case FL_AST_EXPR_RUNARY:
    break;
  case FL_AST_EXPR_CALL: {
    REVERSE_LIST(ast->call.arguments);
  } break;
  case FL_AST_DTOR_VAR:
    break;
  case FL_AST_DECL_FUNCTION: {
    REVERSE_LIST(ast->func.params);
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
}

bool fl_ast_parent_cb(fl_ast_t* node, fl_ast_t* parent, size_t level,
                      void* userdata) {
  node->parent = parent;

  return true;
}

void fl_ast_parent(fl_ast_t* root) {
  fl_ast_traverse(root, fl_ast_parent_cb, 0, 0, 0);
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
    if (ast->program.core) {
      fl_ast_delete(ast->program.core);
    }
    fl_tokens_delete(ast->program.tokens);
    st_delete(&ast->program.code);
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
  case FL_AST_EXPR_CALL: {
    if (ast->call.callee) {
      fl_ast_delete(ast->call.callee);
    }

    if (ast->call.arguments) {
      fl_ast_delete_list(ast->call.arguments);
      ast->call.arguments = 0;
    }

  } break;
  case FL_AST_LIT_STRING:
    st_delete(&ast->string.value);
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

    if (ast->func.ret_type) {
      fl_ast_delete(ast->func.ret_type);
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
  case FL_AST_PARAMETER:
    if (ast->param.id) {
      fl_ast_delete(ast->param.id);
      ast->param.id = 0;
    }

    if (ast->param.type) {
      fl_ast_delete(ast->param.type);
      ast->param.type = 0;
    }
    break;
  case FL_AST_STMT_RETURN: {
    if (ast->ret.argument) {
      fl_ast_delete(ast->ret.argument);
      ast->ret.argument = 0;
    }
  } break;
  case FL_AST_CAST: {
    if (ast->cast.to) {
      fl_ast_delete(ast->cast.to);
    }

    if (ast->cast.right) {
      fl_ast_delete(ast->cast.right);
    }
  } break;
  default: {}
  }
  free(ast);
}

fl_ast_t* fl_ast_search_decl_var(fl_ast_t* node, string* name) {
  while ((node = node->parent) != 0) {
    switch (node->type) {
    case FL_AST_DECL_FUNCTION: {
      size_t i = 0;
      fl_ast_t* tmp;

      if (node->func.params) {
        while ((tmp = node->func.params[i++]) != 0) {
          if (st_cmp(name, tmp->param.id->identifier.string) == 0) {
            printf("param found %zu\n", i);
            return tmp;
          }
        }
      }
    } break;
    case FL_AST_BLOCK: {
      // search in the list
      size_t i = 0;
      fl_ast_t* tmp;

      if (node->block.body) {
        while ((tmp = node->block.body[i++]) != 0) {
          if (tmp->type == FL_AST_DTOR_VAR &&
              st_cmp(name, tmp->var.id->identifier.string) == 0) {
            printf("dtor found  @ %zu index\n", i);
            return tmp;
          }
        }
      }
    }
    default: {} // remove warn
    }
  }

  return 0;
}

size_t fl_ast_get_typeid(fl_ast_t* node) {
  // check AST is somewhat "type-related"
  switch (node->type) {

  case FL_AST_EXPR_CALL: {
    // search function
  }
  case FL_AST_EXPR_BINOP: {
    return fl_ast_get_typeid(node->parent); // TODO REVIEW!!
  }
  case FL_AST_EXPR_ASSIGNAMENT: {
    return fl_ast_get_typeid(node->assignament.left);
  }
  case FL_AST_LIT_IDENTIFIER: {
    printf("identifier: %s\n", node->identifier.string->value);
    // search var-dtor and return it
    fl_ast_t* dtor = fl_ast_search_decl_var(node, node->identifier.string);
    if (dtor) {
      return fl_ast_get_typeid(dtor);
    }
  } break;
  case FL_AST_TYPE:
    printf("type: %zu\n", node->ty.id);
    return node->ty.id;
  case FL_AST_DTOR_VAR:
    printf("dtor: %zu\n", node->var.type->ty.id);
    return node->var.type->ty.id;
  default: {}
  }
  fprintf(stderr, "ast is not type related! %d", node->type);
  exit(1);
}

bool fl_ast_is_pointer(fl_ast_t* node) {
  size_t id = fl_ast_get_typeid(node);
  return fl_type_table[id].of == FL_POINTER;
}

size_t fl_ast_ret_type(fl_ast_t* node) {
  switch (node->type) {
  case FL_AST_EXPR_ASSIGNAMENT:
    return fl_ast_ret_type(node->assignament.right);
  case FL_AST_LIT_NUMERIC:
    return node->numeric.ty_id;
  default: {
    printf("cannot find type!");
    exit(1);
  }
  }

  return 0;
}
