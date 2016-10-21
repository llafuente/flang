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
#include "flang/libparser.h"

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

void ast_dump_one(ast_t* node, FILE* where) {
  fl_assert(node != 0);

  switch (node->type) {
  case AST_PROGRAM:
    fprintf(where, "program [path='%s']", node->program.file->value);
    // too much??
    // printf("\n%s\n", node->program.code->value);
    break;
  case AST_IMPORT:
    fprintf(where, "import [path='%s' forward=%d]",
            node->import.path->string.value->value, node->import.forward);
    break;
  case AST_MODULE:
    fprintf(where, "module [path='%s']", node->program.file->value);
    break;
  case AST_BLOCK:
    // traverse do not follow scope hashes
    // so we print it here
    fprintf(where, "block [%d]", node->block.scope);
    fprintf(where, " types [%s]", __ast_block_hash_append(node->block.types));
    fprintf(where, " vars [%s]",
            __ast_block_hash_append(node->block.variables));
    fprintf(where, " fns [%s]", __ast_block_hash_append(node->block.functions));
    if (node->block.modules.length) {
      fprintf(where, " modules [%lu][", node->block.modules.length);
      for (u64 i = 0; i < node->block.modules.length; ++i) {
        fprintf(where, "%s,",
                ((ast_t*)node->block.modules.values[i])->program.file->value);
      }
      fprintf(where, "]");
    }
    break;
  case AST_LIST:
    fprintf(where, "list [count=%zu]", node->list.length);
    break;
  case AST_EXPR_ASSIGNAMENT:
    fprintf(where, "assignament T(%zu)", node->ty_id);
    break;
  case AST_EXPR_BINOP:
    if (node->binop.operator<127) {
      fprintf(where, "binop T(%zu) [operator=%c]", node->ty_id,
              node->binop.operator);
    } else {
      fprintf(where, "binop T(%zu) [operator=%d]", node->ty_id,
              node->binop.operator);
    }
    break;
  case AST_LIT_INTEGER:
    fprintf(where, "integer T(%zu) [u=%ld] [zu=%zu]", node->ty_id,
            node->integer.signed_value, node->integer.unsigned_value);
    break;
  case AST_LIT_FLOAT:
    fprintf(where, "float T(%zu) [f=%f]", node->ty_id, node->decimal.value);
    break;
  case AST_LIT_IDENTIFIER:
    fprintf(where, "identifier T(%zu) [resolve=%d string=%s]", node->ty_id,
            node->identifier.resolve, node->identifier.string->value);
    break;
  case AST_LIT_STRING:
    fprintf(where, "string T(%zu) [string=%s]", node->ty_id,
            node->string.value->value);
    break;
  case AST_LIT_BOOLEAN:
    fprintf(where, "boolean T(%zu) [value=%d]", node->ty_id,
            node->boolean.value);
    break;
  case AST_EXPR_LUNARY:
    if (node->lunary.operator<127) {
      fprintf(where, "lunary T(%zu) [operator=%c]", node->ty_id,
              node->lunary.operator);
    } else {
      fprintf(where, "lunary T(%zu) [operator=%d]", node->ty_id,
              node->lunary.operator);
    }
    break;
  case AST_EXPR_RUNARY:
    fprintf(where, "runary T(%zu) [operator=%s]", node->ty_id,
            psr_operator_str(node->runary.operator));
    break;
  case AST_EXPR_CALL:
    fprintf(where, "call T(%zu) [arguments=%zu]", node->ty_id,
            node->call.arguments->list.length);
    break;
  case AST_EXPR_MEMBER:
    fprintf(where, "member T(%zu) idx(%zu) expression(%d)", node->ty_id,
            node->member.idx, node->member.expression);
    break;
  case AST_DTOR_VAR:
    fprintf(where, "variable T(%zu) scope(%s)", node->ty_id,
            node->var.scope == AST_SCOPE_BLOCK ? "block" : "global");
    break;
  case AST_TYPE:
    fprintf(where, "type T(%zu) %s", node->ty_id,
            ty_to_string(node->ty_id)->value);
    break;
  case AST_DECL_STRUCT:
    fprintf(where, "struct T(%zu) tpl(%d)", node->ty_id,
            node->structure.templated);
    break;
  case AST_DECL_STRUCT_FIELD:
    fprintf(where, "field T(%zu)", node->ty_id);
    break;
  case AST_DECL_STRUCT_ALIAS:
    fprintf(where, "alias T(%zu)", node->ty_id);
    break;
  case AST_DECL_FUNCTION:
    fprintf(where, "function T(%zu) id(%s) uid(%s) ffi(%d) varargs(%d) tpl(%d) "
                   "[params=%zu]",
            node->ty_id, node->func.id->identifier.string->value,
            node->func.uid ? node->func.uid->value : "(nil)", node->func.ffi,
            node->func.varargs, node->func.templated,
            node->func.params->list.length);
    break;
  case AST_DECL_TEMPLATE:
    fprintf(where, "template");
    break;
  case AST_PARAMETER:
    fprintf(where, "parameter");
    break;
  case AST_STMT_RETURN:
    fprintf(where, "return T(%zu)", node->ty_id);
    break;
  case AST_ERROR:
    fprintf(where, "ERROR [%s: %s]", node->err.type->value,
            node->err.message->value);
    break;
  case AST_STMT_COMMENT: {
    string* quoted = st_escape(node->comment.text);
    fprintf(where, "comment: %s", quoted->value);
    // fprintf(where, "comment");
  } break;
  case AST_STMT_IF:
    fprintf(where, "if");
    break;
  case AST_STMT_LOOP:
    fprintf(where, "loop");
    break;
  case AST_EXPR_SIZEOF:
    fprintf(where, "sizeof T(%zu)", node->ty_id);
    break;
  case AST_EXPR_TYPEOF:
    fprintf(where, "typeof T(%zu)", node->ty_id);
    break;
  case AST_STMT_LOG:
    fprintf(where, "log");
    break;
  case AST_CAST:
    fprintf(where, "cast T(%zu) O(%u)", node->ty_id, node->cast.operation);
    break;
  case AST_IMPLEMENT:
    fprintf(where, "implement");
    break;
  case AST_ATTRIBUTE:
    fprintf(where, "attribute");
    break;
  case AST_NEW:
    fprintf(where, "new");
    break;
  case AST_DELETE:
    fprintf(where, "delete");
    break;
  case AST_COMPILER_ERROR:
    fprintf(where, "compiler-error");
    break;
  default: {}
  }
}

ast_action_t __ast_dump_cb(AST_CB_T_HEADER) {
  if (mode == AST_TRAV_LEAVE)
    return 0;

  if (!node) {
    log_warning("ast_dump: null\n");
    return true;
  }
  level = level * 2;

  // indent
  printf("%*s\x1B[32m @%s id[%lu] ", (int)level, " ", property, node->id);

  ast_dump_one(node, stdout);

  if (node->first_line) {
    printf("\x1B[39m@[%d:%d - %d:%d]", node->first_line, node->first_column,
           node->last_line, node->last_column);
  }

  printf("\x1B[39m\n");
  return AST_SEARCH_CONTINUE;
}

ast_action_t __ast_mindump_cb(AST_CB_T_HEADER) {
  if (mode == AST_TRAV_LEAVE)
    return 0;

  if (!node) {
    log_warning("ast_dump: null\n");
    return true;
  }

  if (node->type == AST_MODULE) {
    return AST_SEARCH_SKIP;
  }

  return __ast_dump_cb(mode, node, parent, level, userdata_in, userdata_out, 0);
}

void ast_mindump(ast_t* node) {
  if (log_debug_level > 2) {
    ast_traverse(node, __ast_mindump_cb, 0, 0, 0, 0);
  }
}

ast_action_t __ast_fulldump_cb(AST_CB_T_HEADER) {
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
      __ast_dump_cb(mode, node, parent, level, userdata_in, userdata_out, 0);

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
