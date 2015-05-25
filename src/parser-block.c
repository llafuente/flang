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

FL_READER_IMPL(block) {
  FL_AST_START(FL_AST_BLOCK);

  if (!FL_ACCEPT_TOKEN(FL_TK_LCBRACKET)) {
    FL_PARSER_ERROR(ast, "expected '{'");
    return ast;
  }

  FL_EXTEND(ast, block_body);
  if (ast->type == FL_AST_ERROR) {
    return ast;
  }

  if (!FL_ACCEPT_TOKEN(FL_TK_RCBRACKET)) {
    FL_PARSER_ERROR(ast, "expected '}'");
    return ast;
  }

  FL_AST_END();
  return ast;
}
FL_READER_IMPL(program_block) {
  FL_AST_START(FL_AST_BLOCK);

  FL_EXTEND(ast, block_body);

  FL_AST_END();
  return ast;
}

fl_reader_cb_t block_stmts[] = {FL_READER_FN(decl_variable),
                                FL_READER_FN(decl_function),
                                FL_READER_FN(expression)};

void FL_READER_FN(block_body)(FL_READER_HEADER, fl_ast_t* extend) {
  fl_ast_t* ast;
  fl_ast_t** list = calloc(100, sizeof(fl_ast_t*));
  extend->block.body = list;
  size_t i = 0;
  size_t j = 0;

  fl_token_t* last_token;

  while (!fl_parser_eof(tokens, state)) {
    last_token = state->token;

    for (j = 0; j < 3; ++j) {
      fl_parser_look_ahead(stack, state);

      ast = block_stmts[j](FL_READER_HEADER_SEND);

      // soft error
      if (!ast) {
        fl_parser_rollback(stack, state);
        continue;
      }
      list[i++] = ast;

      // hard error
      if (ast->type == FL_AST_ERROR) {
        extend->type = FL_AST_ERROR;
        extend->err.str = ast->err.str;
        extend->token_end = ast->token_end;

        // free each list
        fl_ast_delete_list(list);
        fl_parser_rollback(stack, state);
        return;
      }

      fl_parser_commit(stack, state);
      break;
    }

    // read "semicolon" or NEW-LINE
    fl_parser_skipws(tokens, state);
    FL_ACCEPT_TOKEN(FL_TK_SEMICOLON);
    fl_parser_skipws(tokens, state);

    if (state->token->type == FL_TK_RCBRACKET) {
      break;
    }

    if (last_token == state->token) {
      FL_PARSER_ERROR(extend, "unkown statement");
      return;
    }
  }

  printf("block body has %zu elements\n", i);
}
