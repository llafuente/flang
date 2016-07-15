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
#include "flang/debug.h"

char __ast_cbuffer[1024];
void __ast_block_hash_append_cb(char* key, void* decl) {
  strcat(__ast_cbuffer, key);
  strcat(__ast_cbuffer, ",");
}
char* __ast_block_hash_append(hash_t* ht) {
  __ast_cbuffer[0] = 0;
  hash_each(ht, __ast_block_hash_append_cb);
  return __ast_cbuffer;
}

void ast_dump_one(ast_t* node) {
  assert(node != 0);

  switch (node->type) {
  case AST_PROGRAM:
    printf("program [path='%s']", node->program.file->value);
    // too much??
    // printf("\n%s\n", node->program.code->value);
    break;
  case AST_IMPORT:
    printf("import [path='%s' forward=%d]",
           node->import.path->string.value->value, node->import.forward);
    break;
  case AST_MODULE:
    printf("module [path='%s']", node->program.file->value);
    break;
  case AST_BLOCK:
    // traverse do not follow scope hashes
    // so we print it here
    printf("block [%d]", node->block.scope);
    printf(" tys [%s]", __ast_block_hash_append(node->block.types));
    printf(" vars [%s]", __ast_block_hash_append(node->block.variables));
    printf(" fns [%s]", __ast_block_hash_append(node->block.functions));
    break;
  case AST_LIST:
    printf("list [count=%zu]", node->list.count);
    break;
  case AST_EXPR_ASSIGNAMENT:
    printf("assignament T(%zu)", node->ty_id);
    break;
  case AST_EXPR_BINOP:
    if (node->binop.operator<127) {
      printf("binop T(%zu) [operator=%c]", node->ty_id, node->binop.operator);
    } else {
      printf("binop T(%zu) [operator=%d]", node->ty_id, node->binop.operator);
    }
    break;
  case AST_LIT_INTEGER:
    printf("integer T(%zu) [u=%ld] [zu=%zu]", node->ty_id,
           node->integer.signed_value, node->integer.unsigned_value);
    break;
  case AST_LIT_FLOAT:
    printf("float T(%zu) [f=%f]", node->ty_id, node->decimal.value);
    break;
  case AST_LIT_IDENTIFIER:
    printf("identifier T(%zu) [resolve=%d string=%s]", node->ty_id,
           node->identifier.resolve, node->identifier.string->value);
    break;
  case AST_LIT_STRING:
    printf("string T(%zu) [string=%s]", node->ty_id, node->string.value->value);
    break;
  case AST_LIT_BOOLEAN:
    printf("boolean T(%zu) [value=%d]", node->ty_id, node->boolean.value);
    break;
  case AST_EXPR_LUNARY:
    if (node->lunary.operator<127) {
      printf("lunary T(%zu) [operator=%c]", node->ty_id, node->lunary.operator);
    } else {
      printf("lunary T(%zu) [operator=%d]", node->ty_id, node->lunary.operator);
    }
    break;
  case AST_EXPR_RUNARY:
    if (node->runary.operator<127) {
      printf("runary T(%zu) [operator=%d]", node->ty_id, node->runary.operator);
    } else {
      printf("runary T(%zu) [operator=%c]", node->ty_id, node->runary.operator);
    }
    break;
  case AST_EXPR_CALL:
    printf("call T(%zu) [arguments=%zu]", node->ty_id, node->call.narguments);
    break;
  case AST_EXPR_MEMBER:
    printf("member T(%zu) idx(%zu) expression(%d)", node->ty_id,
           node->member.idx, node->member.expression);
    break;
  case AST_DTOR_VAR:
    printf("variable T(%zu) scope(%s)", node->ty_id,
           node->var.scope == AST_SCOPE_BLOCK ? "block" : "global");
    break;
  case AST_TYPE:
    printf("type T(%zu)", node->ty_id);
    break;
  case AST_DECL_STRUCT:
    printf("struct T(%zu)", node->ty_id);
    break;
  case AST_DECL_STRUCT_FIELD:
    printf("field T(%zu)", node->ty_id);
    break;
  case AST_DECL_FUNCTION:
    printf("function T(%zu) id(%s) uid(%s) ffi(%d) varargs(%d) tpl(%d) "
           "[params=%zu]",
           node->ty_id, node->func.id->identifier.string->value,
           node->func.uid ? node->func.uid->value : "(nil)", node->func.ffi,
           node->func.varargs, node->func.templated,
           node->func.params->list.count);
    break;
  case AST_DECL_TEMPLATE:
    printf("template");
    break;
  case AST_PARAMETER:
    printf("parameter");
    break;
  case AST_STMT_RETURN:
    printf("return");
    break;
  case AST_ERROR:
    printf("ERROR [%s: %s]", node->err.type->value, node->err.message->value);
    break;
  case AST_STMT_COMMENT:
    printf("comment\n**\n%s\n**\n", node->comment.text->value);
    // printf("comment");
    break;
  case AST_STMT_IF:
    printf("if");
    break;
  case AST_STMT_LOOP:
    printf("loop");
    break;
  case AST_EXPR_SIZEOF:
    printf("sizeof T(%zu)", node->ty_id);
    break;
  case AST_STMT_LOG:
    printf("log");
    break;
  case AST_CAST:
    printf("cast T(%zu) O(%u)", node->ty_id, node->cast.operation);
    break;
  case AST_ATTRIBUTE:
    printf("attribute");
    break;
  default: {}
  }
}

ast_action_t __ast_dump_cb(ast_trav_mode_t mode, ast_t* node, ast_t* parent,
                           u64 level, void* userdata_in, void* userdata_out) {
  if (mode == AST_TRAV_LEAVE)
    return 0;

  if (!node) {
    log_warning("ast_dump: null\n");
    return true;
  }
  level = level * 2;

  // indent
  printf("%*s• \x1B[32m", (int)level, " ");

  ast_dump_one(node);

  if (node->first_line) {
    printf("\x1B[39m@[%d:%d - %d:%d]", node->first_line, node->first_column,
           node->last_line, node->last_column);
  }

  printf("\x1B[39m\n");
  return AST_SEARCH_CONTINUE;
}

ast_action_t __ast_mindump_cb(ast_trav_mode_t mode, ast_t* node, ast_t* parent,
                              u64 level, void* userdata_in,
                              void* userdata_out) {
  if (mode == AST_TRAV_LEAVE)
    return 0;

  if (!node) {
    log_warning("ast_dump: null\n");
    return true;
  }

  if (node->type == AST_MODULE) {
    return AST_SEARCH_SKIP;
  }

  return __ast_dump_cb(mode, node, parent, level, userdata_in, userdata_out);
}

void ast_mindump(ast_t* node) {
  if (log_debug_level > 2) {
    ast_traverse(node, __ast_mindump_cb, 0, 0, 0, 0);
  }
}

ast_action_t __ast_fulldump_cb(ast_trav_mode_t mode, ast_t* node, ast_t* parent,
                               u64 level, void* userdata_in,
                               void* userdata_out) {
  if (mode == AST_TRAV_LEAVE)
    return 0;

  if (!node) {
    log_warning("ast_dump: null\n");
    return true;
  }

  if (node->type == AST_MODULE) {
    return AST_SEARCH_SKIP;
  }

  ast_action_t t =
      __ast_dump_cb(mode, node, parent, level, userdata_in, userdata_out);

  printf("\n\x1B[33m%s\x1B[39m\n", ast_get_code(node)->value);
  return t;
}

void ast_fulldump(ast_t* node) {
  if (log_debug_level > 3) {
    ast_traverse(node, __ast_fulldump_cb, 0, 0, 0, 0);
  }
}
void ast_dump(ast_t* node) {
  if (log_debug_level > 2) {
    ast_traverse(node, __ast_dump_cb, 0, 0, 0, 0);
  }
}

void ast_dump_s(ast_t* node) {
  int x = log_debug_level;
  log_debug_level = 10;
  ast_traverse(node, __ast_dump_cb, 0, 0, 0, 0);

  log_debug_level = x;
}
