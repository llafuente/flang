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

bool psr_next(tk_token_list_t* tokens, fl_psrstate_t* state) {
  if (psr_eof(tokens, state)) {
    return false;
  }

  state->prev_token = &tokens->tokens[state->current];
  ++state->current;
  state->token = &tokens->tokens[state->current];
  state->next_token = &tokens->tokens[state->current + 1];

  return true;
}

bool psr_prev(tk_token_list_t* tokens, fl_psrstate_t* state) {
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

bool psr_eof(tk_token_list_t* tokens, fl_psrstate_t* state) {
  return state->next_token->type == FL_TK_EOF;
}

bool psr_accept(tk_token_list_t* tokens, fl_psrstate_t* state, char* text) {
  if (strcmp(state->token->string->value, text) == 0) {
    psr_next(tokens, state);
    return true;
  }
  return false;
}

bool psr_accept_list(tk_token_list_t* tokens, fl_psrstate_t* state,
                     char* text[], size_t text_count) {
  size_t i = 0;
  for (; i < text_count; ++i) {
    if (strcmp(state->token->string->value, text[i]) == 0) {
      psr_next(tokens, state);
      return true;
    }
  }
  return false;
}

bool psr_accept_token(tk_token_list_t* tokens, fl_psrstate_t* state,
                      tk_tokens_t token_type) {
  dbg(true, 11, "[%u == %u]", state->token->type, token_type);

  if (state->token->type == token_type) {
    psr_next(tokens, state);
    return true;
  }
  return false;
}

bool psr_accept_token_list(tk_token_list_t* tokens, fl_psrstate_t* state,
                           tk_tokens_t token_type[], size_t tk_count) {
  size_t i = 0;
  for (; i < tk_count; ++i) {
    if (state->token->type == token_type[i]) {
      psr_next(tokens, state);
      return true;
    }
  }
  return false;
}

// maybe: 00A0 FEFF
void psr_skipws(tk_token_list_t* tokens, fl_psrstate_t* state) {
  char* itr;
  char c;
  do {
    itr = state->token->string->value;
    while ((c = *itr) && (c == ' ' || c == '\n' || c == '\r' || c == '\t' ||
                          c == '\v' || c == '\f')) {
      ++itr;
    };

    if (*itr == '\0') {
      psr_next(tokens, state);

      if (psr_eof(tokens, state)) {
        return;
      }
      continue;
    }
    return;
  } while (true);
}

ast_t* psr_read_list(psr_read_t* arr, size_t length, PSR_READ_HEADER) {
  ast_t* target;

  size_t i;
  for (i = 0; i < length; ++i) {

    psr_look_ahead(stack, state);
    target = arr[i](tokens, stack, state);
    log_debug("psr_read_list: %zu [%p]\n", i, target);
    if (!target || target->type == FL_AST_ERROR) {
      psr_rollback(stack, state);
      continue;
    } else {
      psr_commit(stack, state);
      return target;
    }
  }

  return 0;
}
