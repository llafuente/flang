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
    FL_PARSER_ERROR("expected '{'");
  }

  ast->block.body = FL_READ(block_body);

  if (!FL_ACCEPT_TOKEN(FL_TK_RCBRACKET)) {
    FL_PARSER_ERROR("expected '}'");
  }

  FL_AST_END();
  return ast;
}
FL_READER_IMPL(program_block) {
  FL_AST_START(FL_AST_BLOCK);

  ast->block.body = FL_READ(block_body);

  FL_AST_END();
  return ast;
}


FL_LIST_READER_IMPL(block_body) {
  fl_ast_t* ast;
  fl_ast_t** list = calloc(100, sizeof(fl_ast_t*));
  size_t i = 0;

  fl_token_t* last_token;

  while (!fl_parser_eof(tokens, state)) {
    last_token = state->token;

    __FL_TRY_READ(decl_variable);
    if (ast) {
      printf("decl_variable [%p]\n", ast);
      list[i++] = ast;
      goto body_end_read;
    }

    __FL_TRY_READ(decl_function);

    if (ast) {
      printf("decl_function [%p]\n", ast);
      list[i++] = ast;
      goto body_end_read;
    }

    __FL_TRY_READ(expression);
    if (ast) {
      printf("expression! [%p]\n", ast);
      list[i++] = ast;
      goto body_end_read;
    }

  body_end_read:
    // read "semicolon" or NEW-LINE
    fl_parser_skipws(tokens, state);
    FL_ACCEPT_TOKEN(FL_TK_SEMICOLON);
    fl_parser_skipws(tokens, state);

    if (last_token == state->token) {
      fprintf(stderr, "unkown statement @[%d:%d]\n", state->token->start.line,
              state->token->start.column);
      exit(1);
    }
  }

  printf("block body has %d elements\n", i);

  return list;
}
