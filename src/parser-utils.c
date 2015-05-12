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

bool fl_parser_next(fl_token_list_t* tokens, fl_psrstate_t* state) {
  if (tokens->size > state->current + 1) {
    state->prev_token = &tokens->tokens[state->current];
    ++state->current;
    state->token = &tokens->tokens[state->current];
    if (tokens->size > state->current + 1) {
      state->next_token = &tokens->tokens[state->current + 1];
    } else {
      state->next_token = 0;
    }
    return true;
  }

  return false;
}

bool fl_parser_prev(fl_token_list_t* tokens, fl_psrstate_t* state) {
  if (state->current > 0) {
    state->next_token = state->token;
    --state->current;
    state->token = &tokens->tokens[state->current];
    if (state->current > 0) {
      state->prev_token = &tokens->tokens[state->current - 1];
    } else {
      state->prev_token = 0;
    }
    return true;
  }

  return false;
}

bool fl_parser_eof(fl_token_list_t* tokens, fl_psrstate_t* state) {
  return tokens->size == state->current + 1;
}

bool fl_parser_accept(fl_token_list_t* tokens, fl_psrstate_t* state,
                      char* text) {
  if (strcmp(state->token->value->value, text) == 0) {
    fl_parser_next(tokens, state);
    return true;
  }
  return false;
}

fl_parser_result_t* fl_parser_expect(fl_token_list_t* tokens,
                                     fl_psrstate_t* state, char* text,
                                     char* err_msg, bool final) {
  if (fl_parser_accept(tokens, state, text)) {
    return 0;
  }

  fl_parser_result_t* err = malloc(sizeof(fl_parser_result_t));
  err->text = err_msg;

  if (final) {
    err->level = FL_PR_ERROR_FINAL;
  } else {
    err->level = FL_PR_ERROR;
  }

  return err;
}

void fl_parser_skipws(fl_token_list_t* tokens, fl_psrstate_t* state) {
  char* itr;
  char c;
  do {
    itr = state->token->value->value;
    while ((c = *itr) && (c == ' ' || c == '\n')) {
      ++itr;
    };

    if (*itr == '\0') {
      fl_parser_next(tokens, state);

      if (fl_parser_eof(tokens, state)) {
        return;
      }
      continue;
    }
    return;
  } while (true);
}
