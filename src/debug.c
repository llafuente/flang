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

int dbg_debug_level = 4;

void fl_print_type(size_t ty_id) {
  fl_type_t ty = fl_type_table[ty_id];

  switch (ty.of) {
  case FL_VOID:
    printf("[%zu] VOID\n", ty_id);
    break;
  case FL_NUMBER:
    printf("[%zu] Number (fp %d, bits %d, sign %d)\n", ty_id, ty.number.fp,
           ty.number.bits, ty.number.sign);
    break;
  case FL_POINTER:
    printf("[%zu] Pointer -> ", ty_id);
    fl_print_type(ty.ptr.to);
    break;
  case FL_VECTOR:
    printf("[%zu] Vector -> ", ty_id);
    fl_print_type(ty.vector.to);
    break;
  case FL_FUNCTION:
    printf("[%zu] Function -> ", ty.fn.id ? ty.fn.id->value : "Anonymous");
    size_t i;
    fl_print_type(ty.fn.ret);
    for (i = 0; i < ty.fn.nparams; ++i) {
      fl_print_type(ty.fn.params[i]);
    }
  }
}

void fl_print_type_table() {
  size_t i;

  for (i = 0; i < fl_type_size; ++i) {
    fl_print_type(i);
  }
}

bool fl_ast_debug_cb(fl_ast_t* node, fl_ast_t* parent, size_t level,
                     void* userdata) {
  if (!node) {
    printf("(null)\n");
    return true;
  }
  level = level * 2;

  // indent
  printf("%*s-\x1B[32m", (int)level, " ");

  switch (node->type) {
  case FL_AST_PROGRAM:
    printf("program [core=@%p]\n", node->program.core);
    printf("%s\n", node->program.code->value);
    break;
  case FL_AST_BLOCK:
    printf("block");
    break;
  case FL_AST_LIST:
    printf("list [count=%zu]", node->list.count);
    break;
  case FL_AST_EXPR_ASSIGNAMENT:
    printf("assignament");
    break;
  case FL_AST_EXPR_BINOP:
    printf("binop T(%zu) [operator=%d]", node->ty_id, node->binop.operator);
    break;
  case FL_AST_LIT_NUMERIC:
    printf("number T(%zu) [value=%f]", node->ty_id, node->numeric.value);
    break;
  case FL_AST_LIT_IDENTIFIER:
    printf("identifier T(%zu) [string=%s]", node->ty_id,
           node->identifier.string->value);
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
  case FL_AST_DTOR_VAR:
    printf("variable");
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
    printf("function T(%zu) [params=%zu]", node->ty_id, node->func.nparams);
    break;
  case FL_AST_PARAMETER:
    printf("parameter");
    break;
  case FL_AST_STMT_RETURN:
    printf("return");
    break;
  case FL_AST_ERROR:
    printf("ERROR [string=%s]", node->err.str);
    break;
  case FL_AST_STMT_COMMENT:
    // printf("comment\n**\n%s\n**\n", node->comment.text->value);
    printf("comment");
    break;
  case FL_AST_STMT_IF:
    printf("if");
    break;
  case FL_AST_STMT_LOOP:
    printf("loop");
    break;
  case FL_AST_CAST:
    printf("cast T(%zu)", node->ty_id);
    break;
  default: {}
  }

  if (node->token_start && node->token_end) {
    printf("\x1B[36m(%d)[@%p][%3zu:%3zu - %3zu:%3zu]\x1B[39m\n", node->type,
           node, node->token_start->start.column, node->token_start->start.line,
           node->token_end->end.column, node->token_end->end.line);
  } else {
    printf("\x1B[36m(%d)[@%p]\x1B[39m\n", node->type, node);
  }

  return true;
}

void fl_ast_debug(fl_ast_t* node) {
  fl_ast_traverse(node, fl_ast_debug_cb, 0, 0, 0);
}
