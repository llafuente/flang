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

void ast_dump_one(ast_t* node) {
  assert(node != 0);

  if (log_debug_level < 2) {
    return;
  }

  switch (node->type) {
  case FL_AST_PROGRAM:
    printf("program [path='%s']", node->program.file);
    // too much??
    // printf("\n%s\n", node->program.code->value);
    break;
  case FL_AST_IMPORT:
    printf("import [path='%s']", node->import.path->string.value->value);
    break;
  case FL_AST_MODULE:
    printf("module [path='%s']", node->program.file);
    break;
  case FL_AST_BLOCK:
    printf("block");
    break;
  case FL_AST_LIST:
    printf("list [count=%zu]", node->list.count);
    break;
  case FL_AST_EXPR_ASSIGNAMENT:
    printf("assignament T(%zu)", node->ty_id);
    break;
  case FL_AST_EXPR_BINOP:
    printf("binop T(%zu) [operator=%d]", node->ty_id, node->binop.operator);
    break;
  case FL_AST_LIT_INTEGER:
    printf("integer T(%zu) [u=%ld] [zu=%zu]", node->ty_id,
           node->integer.signed_value, node->integer.unsigned_value);
    break;
  case FL_AST_LIT_FLOAT:
    printf("float T(%zu) [f=%f]", node->ty_id, node->decimal.value);
    break;
  case FL_AST_LIT_IDENTIFIER:
    printf("identifier T(%zu) [resolve=%d string=%s]", node->ty_id,
           node->identifier.resolve, node->identifier.string->value);
    break;
  case FL_AST_LIT_STRING:
    printf("string T(%zu) [string=%s]", node->ty_id, node->string.value->value);
    break;
  case FL_AST_LIT_BOOLEAN:
    printf("boolean T(%zu) [value=%d]", node->ty_id, node->boolean.value);
    break;
  case FL_AST_EXPR_LUNARY:
    printf("lunary T(%zu) [operator=%d]", node->ty_id, node->lunary.operator);
    break;
  case FL_AST_EXPR_RUNARY:
    printf("runary T(%zu) [operator=%d]", node->ty_id, node->runary.operator);
    break;
  case FL_AST_EXPR_CALL:
    printf("call T(%zu) [arguments=%zu]", node->ty_id, node->call.narguments);
    break;
  case FL_AST_EXPR_MEMBER:
    printf("member T(%zu) idx(%zu) expression(%d)", node->ty_id,
           node->member.idx, node->member.expression);
    break;
  case FL_AST_DTOR_VAR:
    printf("variable T(%zu)", node->ty_id);
    break;
  case FL_AST_TYPE:
    printf("type T(%zu)", node->ty_id);
    break;
  case FL_AST_DECL_STRUCT:
    printf("struct T(%zu)", node->ty_id);
    break;
  case FL_AST_DECL_STRUCT_FIELD:
    printf("field T(%zu)", node->ty_id);
    break;
  case FL_AST_DECL_FUNCTION:
    printf("function T(%zu) id(%s) uid(%s) [params=%zu]", node->ty_id,
           node->func.id->identifier.string->value,
           node->func.uid ? node->func.uid->value : "(nil)",
           node->func.params->list.count);
    break;
  case FL_AST_PARAMETER:
    printf("parameter");
    break;
  case FL_AST_STMT_RETURN:
    printf("return");
    break;
  case FL_AST_ERROR:
    printf("ERROR [%s: %s]", node->err.type->value, node->err.message->value);
    break;
  case FL_AST_STMT_COMMENT:
    printf("comment\n**\n%s\n**\n", node->comment.text->value);
    // printf("comment");
    break;
  case FL_AST_STMT_IF:
    printf("if");
    break;
  case FL_AST_STMT_LOOP:
    printf("loop");
    break;
  case FL_AST_CAST:
    printf("cast T(%zu) O(%u)", node->ty_id, node->cast.operation);
    break;
  default: {}
  }
}

ast_action_t ast_dump_cb(ast_t* node, ast_t* parent, size_t level,
                         void* userdata_in, void* userdata_out) {
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
  return FL_AC_CONTINUE;
}

void ast_dump(ast_t* node) {
  if (log_debug_level > 2) {
    ast_traverse(node, ast_dump_cb, 0, 0, 0, 0);
  }
}
