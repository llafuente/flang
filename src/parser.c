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

void fl_state_debug(fl_psrstate_t* state) {
  printf("[%s] current: %zu | look_ahead_idx: %zu\n",
         state->token->string->value, state->current, state->look_ahead_idx);
  /*
  size_t ;
  fl_token_t* token;
  fl_token_t* prev_token;
  fl_token_t* next_token;
  size_t ;
  */
}

fl_ast_t* fl_parser(fl_token_list_t* tokens) {
  fl_psrstack_t* stack = malloc(sizeof(fl_psrstack_t));
  fl_psrstate_t* state = malloc(sizeof(fl_psrstate_t));

  fl_parser_stack_init(stack, tokens, state);

  FL_AST_START(FL_AST_PROGRAM);
  ast->program.body = FL_READ(body);

  FL_AST_END();

  free(stack);
  free(state);

  return ast;
}

FL_LIST_READER_IMPL(body) {
  fl_ast_t* ast;
  fl_ast_t** list = calloc(100, sizeof(fl_ast_t*));
  size_t i = 0;

  fl_token_t* last_token;

  while (!fl_parser_eof(tokens, state)) {
    last_token = state->token;

    __FL_TRY_READ(decl_variable);
    if (ast) {
      printf("decl_variable\n");
      list[i++] = ast;
      goto body_end_read;
    }

    __FL_TRY_READ(expression);
    if (ast) {
      printf("expression!");
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

  printf("body has %d elements\n", i);

  return list;
}

fl_ast_t* fl_parse_utf8(char* str) {
  string* code;
  fl_token_list_t* tokens;
  fl_parser_result_t* err;
  // tets priority <= gt than '<' '='
  code = st_newc(str, st_enc_utf8);
  tokens = fl_tokenize(code);

  fl_ast_t* root = fl_parser(tokens);

  fl_tokens_delete(tokens);
  st_delete(&code);

  fl_ast_traverse(root, fl_ast_debug_cb, 0, 0);
#ifdef FL_VERBOSE
#endif

  return root;
}
