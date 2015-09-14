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
#include "tasks.h"

TASK_IMPL(parser_utils) {
  log_debug_level = 0;

  string* code;
  tk_token_list_t* tokens;
  // tets priority <= gt than '<' '='
  code = st_newc("log \"hello:\\\"world\";", st_enc_utf8);
  tokens = fl_tokenize(code);

  ASSERT(tokens->size == 8, "8 tokens parsed (6 + nl + eof)");

  fl_psrstack_t stack;
  fl_psrstate_t state;

  psr_stack_init(&stack, tokens, &state);

  ASSERT(strcmp(state.token->string->value, "log") == 0, "1st token");
  ASSERT(state.current == 0, "1st token id");

  psr_next(tokens, &state);
  ASSERT(strcmp(state.token->string->value, " ") == 0, "2nd token");
  ASSERT(state.current == 1, "2nd token id");

  psr_next(tokens, &state);
  ASSERT(strcmp(state.token->string->value, "\"") == 0, "3rd token");

  psr_next(tokens, &state);
  ASSERT(strcmp(state.token->string->value, "hello:\\\"world") == 0,
         "4th token");

  psr_next(tokens, &state);
  ASSERT(strcmp(state.token->string->value, "\"") == 0, "5th token");
  ASSERT(psr_eof(tokens, &state) == false, "is eof - no");

  psr_next(tokens, &state);
  ASSERT(strcmp(state.token->string->value, ";") == 0, "6th token");
  ASSERT(state.current == 5, "6th token id");

  ASSERT(psr_eof(tokens, &state) == false, "is eof - no (fake nl left)");

  psr_next(tokens, &state);
  ASSERT(psr_eof(tokens, &state) == true, "is eof - yes");
  ASSERT(state.current == 6, "7th token id");

  // overflow - no problem
  psr_next(tokens, &state);
  ASSERT(state.current == 6, "7th token id");
  ASSERT(psr_eof(tokens, &state) == true, "is eof - yes");

  psr_prev(tokens, &state);
  psr_prev(tokens, &state);
  ASSERT(state.current == 4, "5th token id");
  ASSERT(psr_eof(tokens, &state) == false, "is eof - no");

  psr_look_ahead(&stack, &state);
  ASSERT(state.look_ahead_idx == 1, "look ahead is 1");
  psr_prev(tokens, &state);
  ASSERT(strcmp(state.token->string->value, "hello:\\\"world") == 0,
         "4th token");
  psr_rollback(&stack, &state);
  ASSERT(state.current == 4, "5th token id");
  ASSERT(psr_eof(tokens, &state) == false, "is eof - no");
  ASSERT(state.look_ahead_idx == 0, "look ahead is 0");

  psr_look_ahead(&stack, &state);
  psr_prev(tokens, &state);
  ASSERT(state.current == 3, "4th token id");
  psr_commit(&stack, &state);
  ASSERT(state.current == 3, "4th token id");

  ASSERT(psr_accept(tokens, &state, "hello:\\\"world") == true,
         "accept text");
  ASSERT(state.current == 4, "5th token id");
  ASSERT(psr_accept(tokens, &state, "hello:\\\"world") == false,
         "no accept text");
  ASSERT(state.current == 4, "5th token id");

  psr_stack_init(&stack, tokens, &state);

  psr_skipws(tokens, &state);
  ASSERT(state.current == 0, "1st token id, no move (skip)");
  psr_next(tokens, &state);
  ASSERT(state.current == 1, "2nd token id (next)");
  psr_skipws(tokens, &state);
  ASSERT(state.current == 2, "3rd token id, move (skip)");

  tk_tokens_delete(tokens);
  st_delete(&code);

  return 0;
}
