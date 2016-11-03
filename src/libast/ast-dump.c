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
char __ast_cbuffer_len = 0;
void __ast_block_hash_append_cb(char* key, void* decl) {
  __ast_cbuffer_len = sprintf(__ast_cbuffer + __ast_cbuffer_len, "%s(%lu),",
                              key, ((ast_t*)decl)->ty_id);
}
char* __ast_block_hash_append(hash_t* ht) {
  __ast_cbuffer[0] = 0;
  __ast_cbuffer_len = 0;
  hash_each(ht, __ast_block_hash_append_cb);
  return __ast_cbuffer;
}

void __ast_dump_type(ast_t* node) {
  fprintf(ast_dump_file, " \x1B[34mT(%lu)\x1B[39m", node->ty_id);
}
void ast_dump_one(ast_t* node) {
  fl_assert(node != 0);

  switch (node->type) {
  case AST_PROGRAM:
    fprintf(ast_dump_file, "program [path='%s']", node->program.file->value);
    // too much??
    // printf("\n%s\n", node->program.code->value);
    break;
  case AST_IMPORT:
    fprintf(ast_dump_file, "import [path='%s' forward=%d]",
            node->import.path->string.value->value, node->import.forward);
    break;
  case AST_MODULE:
    fprintf(ast_dump_file, "module [path='%s']", node->program.file->value);
    break;
  case AST_BLOCK:
    // traverse do not follow scope hashes
    // so we print it here

    fprintf(ast_dump_file, "\x1B[101mblock\x1B[49m [%d]", node->block.scope);
    fprintf(ast_dump_file, " types [%s]",
            __ast_block_hash_append(node->block.types));
    fprintf(ast_dump_file, " vars [%s]",
            __ast_block_hash_append(node->block.variables));
    fprintf(ast_dump_file, " fns [%s]",
            __ast_block_hash_append(node->block.functions));
    if (node->block.modules.length) {
      fprintf(ast_dump_file, " modules [%lu][", node->block.modules.length);
      for (u64 i = 0; i < node->block.modules.length; ++i) {
        fprintf(ast_dump_file, "%s,",
                ((ast_t*)node->block.modules.values[i])->program.file->value);
      }
      fprintf(ast_dump_file, "]");
    }
    break;
  case AST_LIST:
    fprintf(ast_dump_file, "list [length=%zu]", node->list.length);
    break;
  case AST_EXPR_ASSIGNAMENT:
    fprintf(ast_dump_file, "assignament [operator %s]",
            psr_operator_str(node->assignament.operator));
    __ast_dump_type(node);
    break;
  case AST_EXPR_BINOP:
    fprintf(ast_dump_file, "binop [operator %s]",
            psr_operator_str(node->binop.operator));
    __ast_dump_type(node);
    break;
  case AST_LIT_INTEGER:
    fprintf(ast_dump_file, "integer [signed=%ld] [unsigned=%zu]",
            node->integer.signed_value, node->integer.unsigned_value);
    __ast_dump_type(node);
    break;
  case AST_LIT_FLOAT:
    fprintf(ast_dump_file, "float [value=%f]", node->decimal.value);
    __ast_dump_type(node);
    break;
  case AST_LIT_IDENTIFIER:
    fprintf(ast_dump_file,
            "\x1B[104midentifier\x1B[49m [resolve? %s, string=`%s`]",
            node->identifier.resolve ? "yes" : "no",
            node->identifier.string->value);
    __ast_dump_type(node);
    break;
  case AST_LIT_STRING:
    fprintf(ast_dump_file, "string [string=`%s`]", node->string.value->value);
    __ast_dump_type(node);
    break;
  case AST_LIT_BOOLEAN:
    fprintf(ast_dump_file, "boolean [value=%s]",
            node->boolean.value ? "true" : "false");
    __ast_dump_type(node);
    break;
  case AST_EXPR_LUNARY:
    fprintf(ast_dump_file, "lunary [operator %s]",
            psr_operator_str(node->lunary.operator));
    __ast_dump_type(node);
    break;
  case AST_EXPR_RUNARY:
    fprintf(ast_dump_file, "runary [operator %s]",
            psr_operator_str(node->runary.operator));
    __ast_dump_type(node);
    break;
  case AST_EXPR_CALL:
    fprintf(ast_dump_file, "call [arguments=%zu]",
            node->call.arguments->list.length);
    if (node->call.decl) {
      fprintf(ast_dump_file, "[function=%s]",
              node->call.decl->func.id->identifier.string->value);
    }
    __ast_dump_type(node);
    break;
  case AST_EXPR_MEMBER:
    fprintf(ast_dump_file, "member [index=%zu expression? %s brakets? %s]",
            node->member.idx, (node->member.expression ? "yes" : "no"),
            (node->member.brakets ? "yes" : "no"));
    __ast_dump_type(node);
    break;
  case AST_DTOR_VAR:
    fprintf(ast_dump_file, "variable [scope=%s, scoped? %s]",
            node->var.scope == AST_SCOPE_BLOCK ? "block" : "global",
            node->var.scoped ? "yes" : "no");
    __ast_dump_type(node);
    break;
  case AST_TYPE:
    fprintf(ast_dump_file, "type [%s]", ty_to_string(node->ty_id)->value);
    __ast_dump_type(node);
    break;
  case AST_DECL_STRUCT:
    // REVIEW add more data ?
    fprintf(ast_dump_file, "struct [templated? %s]",
            node->structure.templated ? "yes" : "no");
    __ast_dump_type(node);
    break;
  case AST_DECL_STRUCT_FIELD:
    fprintf(ast_dump_file, "field");
    __ast_dump_type(node);
    break;
  case AST_DECL_STRUCT_ALIAS:
    fprintf(ast_dump_file, "alias");
    __ast_dump_type(node);
    break;
  case AST_DECL_FUNCTION:
    fprintf(
        ast_dump_file,
        "\x1B[102mfunction\x1B[49m [id=`%s`, uid=`%s`, nparams=%zu, ffi? %s, "
        "vararg? %s, templated? %s]",
        node->func.id->identifier.string->value,
        node->func.uid ? node->func.uid->value : "(nil)",
        node->func.params->list.length, node->func.ffi ? "yes" : "no",
        node->func.varargs ? "yes" : "no", node->func.templated ? "yes" : "no");
    __ast_dump_type(node);
    break;
  case AST_DECL_TEMPLATE:
    fprintf(ast_dump_file, "template");
    break;
  case AST_PARAMETER:
    fprintf(ast_dump_file, "parameter");
    break;
  case AST_STMT_RETURN:
    fprintf(ast_dump_file, "return");
    __ast_dump_type(node);
    break;
  case AST_ERROR:
    fprintf(ast_dump_file, "ERROR [%s: %s]", node->err.type->value,
            node->err.message->value);
    break;
  case AST_STMT_COMMENT: {
    string* quoted = st_escape(node->comment.text);
    fprintf(ast_dump_file, "comment: %s", quoted->value);
    // fprintf(ast_dump_file, "comment");
  } break;
  case AST_STMT_IF:
    fprintf(ast_dump_file, "if");
    break;
  case AST_STMT_LOOP:
    fprintf(ast_dump_file, "loop");
    break;
  case AST_EXPR_SIZEOF:
    fprintf(ast_dump_file, "sizeof");
    __ast_dump_type(node);
    break;
  case AST_EXPR_TYPEOF:
    fprintf(ast_dump_file, "typeof");
    __ast_dump_type(node);
    break;
  case AST_STMT_LOG:
    fprintf(ast_dump_file, "log");
    break;
  case AST_CAST:
    fprintf(ast_dump_file, "cast [operation=%u] to %s", node->cast.operation,
            ty_to_string(node->ty_id)->value);
    __ast_dump_type(node);
    break;
  case AST_IMPLEMENT:
    fprintf(ast_dump_file, "implement");
    break;
  case AST_ATTRIBUTE:
    fprintf(ast_dump_file, "attribute");
    break;
  case AST_NEW:
    fprintf(ast_dump_file, "new");
    break;
  case AST_DELETE:
    fprintf(ast_dump_file, "delete");
    break;
  case AST_COMPILER_ERROR:
    fprintf(ast_dump_file, "compiler-error");
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
  // fprintf(ast_dump_file, "\x1B[32m");
  fprintf(ast_dump_file, "%*s @%s ", (int)level, " ", property);
  // fprintf("id[%lu]", node->id);

  ast_dump_one(node);
  // fprintf(ast_dump_file, "\x1B[39m");
  /*
  if (node->first_line) {
    fprintf(ast_dump_file, "@[%d:%d - %d:%d]", node->first_line,
            node->first_column, node->last_line, node->last_column);
  }
  */

  fprintf(ast_dump_file, "\n");
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

  fprintf(ast_dump_file, "\n\x1B[33m%s\x1B[39m\n", ast_get_code(node)->value);
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

void ast_fdump(FILE* f, ast_t* node) {
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
