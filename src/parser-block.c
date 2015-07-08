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
  PSR_AST_START(FL_AST_BLOCK);

  if (!PSR_ACCEPT_TOKEN(FL_TK_LCBRACKET)) {
    PSR_SYNTAX_ERROR(ast, "expected '{'");
    return ast;
  }

  fl_parser_look_ahead(stack, state);
  PSR_AST_EXTEND(ast, block_body);
  if (ast->type == FL_AST_ERROR) {
    fl_parser_rollback(stack, state);

    return ast;
  }

  if (!PSR_ACCEPT_TOKEN(FL_TK_RCBRACKET)) {
    PSR_SYNTAX_ERROR(ast, "expected '}'");
    fl_parser_rollback(stack, state);
    return ast;
  }

  fl_parser_commit(stack, state);

  PSR_AST_RET();
}
PSR_READ_IMPL(program_block) {
  PSR_AST_START(FL_AST_BLOCK);

  PSR_AST_EXTEND(ast, block_body);

  PSR_AST_RET();
}

fl_read_cb_t block_stmts[] = {
    PSR_READ_NAME(decl_variable), PSR_READ_NAME(decl_function),
    PSR_READ_NAME(stmt_return),   PSR_READ_NAME(expression),
    PSR_READ_NAME(stmt_if),       PSR_READ_NAME(comment)};

void PSR_READ_NAME(block_body)(PSR_READ_HEADER, fl_ast_t** extend) {
  fl_ast_t* ast;
  fl_ast_t** list = calloc(100, sizeof(fl_ast_t*));
  (*extend)->block.body = list;
  size_t i = 0;
  size_t j = 0;

  fl_token_t* last_token;

  while (!fl_parser_eof(tokens, state)) {
    last_token = state->token;

    for (j = 0; j < 6; ++j) {
      fl_parser_look_ahead(stack, state);

      ast = block_stmts[j](PSR_READ_HEADER_SEND);

      // soft error
      if (!ast) {
        fl_parser_rollback(stack, state);
        continue;
      }

      // hard error
      if (ast->type == FL_AST_ERROR) {
        // free each list
        fl_ast_delete(*extend);
        *extend = ast;

        fl_parser_rollback(stack, state);
        return;
      }

      list[i++] = ast;

      fl_parser_commit(stack, state);
      break;
    }

    // read "semicolon" or NEW-LINE
    fl_parser_skipws(tokens, state);
    PSR_ACCEPT_TOKEN(FL_TK_SEMICOLON);
    fl_parser_skipws(tokens, state);

    if (state->token->type == FL_TK_RCBRACKET) {
      break;
    }

    if (last_token == state->token) {
      fl_ast_delete_list(list);
      (*extend)->block.body = 0;
      PSR_SYNTAX_ERROR((*extend), "unkown statement");
      return;
    }
  }

  (*extend)->block.nbody = i;
}
