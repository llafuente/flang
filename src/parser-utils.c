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
  if (fl_parser_eof(tokens, state)) {
    return false;
  }

  state->prev_token = &tokens->tokens[state->current];
  ++state->current;
  state->token = &tokens->tokens[state->current];
  state->next_token = &tokens->tokens[state->current + 1];

  return true;
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
  return state->next_token->type == FL_TK_EOF;
}

bool fl_parser_accept(fl_token_list_t* tokens, fl_psrstate_t* state,
                      char* text) {
  if (strcmp(state->token->string->value, text) == 0) {
    fl_parser_next(tokens, state);
    return true;
  }
  return false;
}

bool fl_parser_accept_list(fl_token_list_t* tokens, fl_psrstate_t* state,
                           char* text[], size_t text_count) {
  size_t i = 0;
  for (; i < text_count; ++i) {
    if (strcmp(state->token->string->value, text[i]) == 0) {
      fl_parser_next(tokens, state);
      return true;
    }
  }
  return false;
}

bool fl_parser_accept_token(fl_token_list_t* tokens, fl_psrstate_t* state,
                            fl_tokens_t token_type) {
  if (state->token->type == token_type) {
    fl_parser_next(tokens, state);
    return true;
  }
  return false;
}

bool fl_parser_accept_token_list(fl_token_list_t* tokens, fl_psrstate_t* state,
                                 fl_tokens_t token_type[], size_t tk_count) {
  size_t i = 0;
  for (; i < tk_count; ++i) {
    if (state->token->type == token_type[i]) {
      fl_parser_next(tokens, state);
      return true;
    }
  }
  return false;
}

// maybe: 00A0 FEFF
void fl_parser_skipws(fl_token_list_t* tokens, fl_psrstate_t* state) {
  char* itr;
  char c;
  do {
    itr = state->token->string->value;
    while ((c = *itr) && (c == ' ' || c == '\n' || c == '\r' || c == '\t' ||
                          c == '\v' || c == '\f')) {
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

fl_ast_t* psr_read_list(psr_read_t* arr, size_t length, PSR_READ_HEADER) {
  fl_ast_t* target;

  size_t i;
  for (i = 0; i < length; ++i) {

    fl_parser_look_ahead(stack, state);
    target = arr[i](tokens, stack, state);
    dbg_debug("psr_read_list: %d [%p]\n", i, target);
    if (!target || target->type == FL_AST_ERROR) {
      fl_parser_rollback(stack, state);
      continue;
    } else {
      fl_parser_commit(stack, state);
      return target;
    }
  }

  return 0;
}
