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
         state->token->value->value, state->current, state->look_ahead_idx);
  /*
  size_t ;
  fl_token_t* token;
  fl_token_t* prev_token;
  fl_token_t* next_token;
  size_t ;
  */
}

extern void fl_read_block();
extern void fl_read_body();

void fl_parser(fl_token_list_t* tokens) {
  fl_psrstack_t stack;
  fl_psrstate_t state;
  fl_ast_t ast;

  fl_parser_stack_init(&stack, tokens, &state);

  fl_read_body(&stack, &state, &ast);

  fl_parser_next(tokens, &state);
  fl_state_debug(&state);

  fl_parser_look_ahead(&stack, &state);

  fl_parser_next(tokens, &state);
  fl_parser_next(tokens, &state);
  fl_state_debug(&state);

  fl_parser_commit(&stack, &state);
  fl_state_debug(&state);

  fl_parser_look_ahead(&stack, &state);
  fl_parser_prev(tokens, &state);
  fl_parser_prev(tokens, &state);

  fl_parser_rollback(&stack, &state);
  fl_state_debug(&state);
}

void fl_read_block() {}

void fl_read_body(fl_psrstack_t* stack, fl_psrstate_t* state, fl_ast_t* ast) {}
