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
    log_warning("fl_ast_traverse: (nil)");
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
  case FL_AST_LIST: {
    TRAVERSE_LIST(ast->list.elements);
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
    TRAVERSE(ast->call.arguments);
  } break;
  case FL_AST_EXPR_MEMBER: {
    TRAVERSE(ast->member.left);
    TRAVERSE(ast->member.property);
  } break;
  case FL_AST_DTOR_VAR: {
    TRAVERSE(ast->var.id);
    TRAVERSE(ast->var.type);
  } break;
  case FL_AST_DECL_FUNCTION: {
    TRAVERSE(ast->func.id);
    TRAVERSE(ast->func.ret_type);
    TRAVERSE(ast->func.params);
    TRAVERSE(ast->func.body);
  } break;
  case FL_AST_PARAMETER: {
    TRAVERSE(ast->param.id);
    TRAVERSE(ast->param.type);
  } break;
  case FL_AST_STMT_RETURN: {
    TRAVERSE(ast->ret.argument);
  } break;
  case FL_AST_STMT_IF: {
    TRAVERSE(ast->if_stmt.test);
    TRAVERSE(ast->if_stmt.block);
    TRAVERSE(ast->if_stmt.alternate);
  } break;
  case FL_AST_STMT_LOOP: {
    TRAVERSE(ast->loop.init);
    TRAVERSE(ast->loop.pre_cond);
    TRAVERSE(ast->loop.update);
    TRAVERSE(ast->loop.block);
    TRAVERSE(ast->loop.post_cond);
  } break;
  case FL_AST_CAST: {
    TRAVERSE(ast->cast.element);
  } break;
  case FL_AST_DECL_STRUCT: {
    TRAVERSE(ast->structure.id);
    TRAVERSE(ast->structure.fields);
  } break;
  case FL_AST_DECL_STRUCT_FIELD: {
    TRAVERSE(ast->field.id);
    TRAVERSE(ast->field.type);
  } break;
  default: {}
  }
}

void* fl_ast_reverse(fl_ast_t* ast, fl_ast_ret_cb_t cb, fl_ast_t* parent,
                     size_t level, void* userdata) {
  void* ret;
#define REVERSE(node)                                                          \
  if (node) {                                                                  \
    ret = fl_ast_reverse(node, cb, ast, level, userdata);                      \
    if (ret)                                                                   \
      return ret;                                                              \
  }

#define REVERSE_LIST(node)                                                     \
  {                                                                            \
    size_t i = 0;                                                              \
    fl_ast_t* tmp;                                                             \
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
    log_warning("fl_ast_reverse: (nil)");
    return 0;
  }

  ++level;
  // stop if callback is false
  if (!cb(ast, parent, level, userdata, &ret)) {
    return ret;
  }

  switch (ast->type) {
  case FL_AST_PROGRAM:
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
  fl_ast_delete_props(ast);
  free(ast);
}
void fl_ast_delete_props(fl_ast_t* ast) {

#define SAFE_DEL(test)                                                         \
  if (test) {                                                                  \
    fl_ast_delete(test);                                                       \
    test = 0;                                                                  \
  }

#define SAFE_DEL_LIST(test)                                                    \
  if (test) {                                                                  \
    fl_ast_delete_list(test);                                                  \
    test = 0;                                                                  \
  }

  #define SAFE_DEL_STR(test)                                                         \
    if (test) {                                                                  \
      st_delete(&test);                                                       \
    }

  switch (ast->type) {
  case FL_AST_PROGRAM: {
    SAFE_DEL(ast->program.body);
    SAFE_DEL(ast->program.core);
    fl_tokens_delete(ast->program.tokens);
    SAFE_DEL_STR(ast->program.code);
  } break;
  case FL_AST_BLOCK: {
    SAFE_DEL_LIST(ast->block.body);
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
      fl_ast_delete(ast->func.body);
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
  default: {}
  }
}

fl_ast_t* fl_ast_search_decl_var(fl_ast_t* node, string* name) {
  while ((node = node->parent) != 0) {
    switch (node->type) {
    case FL_AST_DECL_FUNCTION: {

      if (node->func.nparams) {
        size_t i = 0;
        fl_ast_t* tmp;
        fl_ast_t* list = node->func.params;
        while ((tmp = list->list.elements[i++]) != 0) {
          if (st_cmp(name, tmp->param.id->identifier.string) == 0) {
            log_verbose("found parameter @ [%zu]", i);
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
            log_verbose("found var decl @ [%zu]", i);
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
  case FL_AST_DTOR_VAR:
    log_verbose("var decl T(%zu)", node->var.type->ty_id);
    return node->var.type->ty_id;
  case FL_AST_PARAMETER:
    log_verbose("parameter T(%zu)", node->param.id->ty_id);
    return node->param.id->ty_id;
  case FL_AST_EXPR_CALL: {
    // search function
  }
  case FL_AST_EXPR_MEMBER:
  case FL_AST_EXPR_BINOP: {
    // this is valid only after ts_pass
    return node->ty_id;
  }
  case FL_AST_EXPR_ASSIGNAMENT: {
    return fl_ast_get_typeid(node->assignament.left);
  }
  case FL_AST_LIT_IDENTIFIER: {
    if (node->ty_id) {
      return node->ty_id;
    }

    log_verbose("identifier T(%s)", node->identifier.string->value);
    // search var-dtor and return it
    fl_ast_t* dtor = fl_ast_search_decl_var(node, node->identifier.string);
    if (dtor) {
      return fl_ast_get_typeid(dtor);
    }
  } break;
  case FL_AST_TYPE:
    log_verbose("type T(%zu)", node->ty_id);
    return node->ty_id;
  case FL_AST_CAST:
    return node->ty_id;
  case FL_AST_LIT_BOOLEAN:
    return 2; // TODO maybe 3, i8
  case FL_AST_LIT_NUMERIC:
    return node->ty_id;
  // dont give information, continue up
  case FL_AST_EXPR_LUNARY:
    return fl_ast_get_typeid(node->parent);
  case FL_AST_LIT_STRING:
    return 13;
  default: {}
  }
  log_error("fl_ast_get_typeid: node is not type related! %d", node->type);
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
    return node->ty_id;
  default: { log_error("fl_ast_ret_type: cannot find type!"); }
  }

  return 0;
}

bool fl_ast_find_fn_decl_cb(fl_ast_t* node, fl_ast_t* parent, size_t level,
                            void* userdata, void** ret) {

  if (node->type == FL_AST_DECL_FUNCTION) {
    string* id = (string*)userdata;

    if (st_cmp(id, node->func.id->identifier.string) == 0) {
      *ret = node;
      return false;
    }
  }

  return true;
}

fl_ast_t* fl_ast_find_fn_decl(fl_ast_t* identifier) {
  if (identifier->type != FL_AST_LIT_IDENTIFIER) {
    log_error("(fl_ast_find_fn_decl) must be an identifier!");
  }

  return (fl_ast_t*)fl_ast_reverse(identifier, fl_ast_find_fn_decl_cb, 0, 0,
                                   (void*)identifier->identifier.string);
}
