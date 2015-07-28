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

PSR_READ_IMPL(block) {
  PSR_START(block, FL_AST_BLOCK);
  cg_print("start block\n");

  PSR_EXPECT_TOKEN(FL_TK_LCBRACKET, block, {}, "expected '{'");

  PSR_SKIPWS();

  cg_print("{ readed \n");
  fl_parser_look_ahead(stack, state);

  PSR_EXTEND(block, block_body);

  PSR_RET_IF_ERROR(block, { fl_parser_rollback(stack, state); });

  PSR_EXPECT_TOKEN(FL_TK_RCBRACKET, block,
                   { fl_parser_rollback(stack, state); }, "expected '}'");
  cg_print("} readed \n");

  PSR_SKIPWS();

  fl_parser_commit(stack, state);

  cg_print("end block ok\n");

  PSR_RET_OK(block);
}
PSR_READ_IMPL(program_block) {
  PSR_START(block, FL_AST_BLOCK);

  PSR_EXTEND(block, block_body);

  PSR_RET_OK(block);
}

fl_read_cb_t block_stmts[] = {
    PSR_READ_NAME(decl_variable), PSR_READ_NAME(decl_function),
    PSR_READ_NAME(stmt_return),   PSR_READ_NAME(expression),
    PSR_READ_NAME(stmt_if),       PSR_READ_NAME(comment),
    PSR_READ_NAME(stmt_for),      PSR_READ_NAME(stmt_while)};

void PSR_READ_NAME(block_body)(PSR_READ_HEADER, fl_ast_t** extend) {
  fl_ast_t* stmt;
  fl_ast_t** list = calloc(100, sizeof(fl_ast_t*));
  (*extend)->block.body = list;
  size_t i = 0;
  size_t last;
  size_t j = 0;

  while (!fl_parser_eof(tokens, state)) {
    last = i;

    for (j = 0; j < 8; ++j) {
      cg_print("read block id: %zu\n", j);
      fl_parser_look_ahead(stack, state);
      stmt = block_stmts[j](PSR_READ_HEADER_SEND);

      // soft error
      if (!stmt) {
        fl_parser_rollback(stack, state);
        continue;
      }

      fl_ast_debug(stmt);

      // hard error
      if (stmt->type == FL_AST_ERROR) {
        fl_parser_rollback(stack, state);

        // free extended node, and "return" current stmt error
        fl_ast_delete(*extend);
        *extend = stmt;

        return;
      }

      fl_parser_commit(stack, state);
      list[i++] = stmt;

      break;
    }

    // read "semicolon" or NEW-LINE
    PSR_SKIPWS();
    PSR_ACCEPT_TOKEN(FL_TK_SEMICOLON);
    PSR_SKIPWS();

    if (state->token->type == FL_TK_RCBRACKET) {
      break;
    }

    // nothing readed!
    if (last == i) {
      fl_ast_delete_list(list);
      (*extend)->block.body = 0;
      PSR_SET_SYNTAX_ERROR((*extend), "invalid statement");
      return;
    }
  }

  (*extend)->block.nbody = i;
}
