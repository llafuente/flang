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

// until i have a better name...
//#define FL_DBG(...) fprintf(stdout, __VA_ARGS__)
#define FL_DBG(...)

fl_tokens_cfg_t* fl_get_token(char* itr, size_t len) {
  size_t tidx = 0;
  size_t tk_size;

  while (tidx < fl_token_list_count) {
    tk_size = fl_token_list[tidx].text_s;

    if (tk_size < len && memcmp(fl_token_list[tidx].text, itr, tk_size) == 0) {
      return &fl_token_list[tidx];
    }
    ++tidx;
  }

  return 0;
}

size_t fl_get_escape_count(char* itr, char* start) {
  size_t count = 0;

  --itr;
  while (itr > start && *itr == '\\') {
    ++count;
    --itr;
  }

  return count;
}

void fl_tokenize_push(fl_token_list_t* tokens, fl_tokens_t type, char* p,
                      size_t p_s, size_t ltoken_line, size_t ltoken_column,
                      size_t line, size_t column) {
  size_t tokens_s = tokens->size;

  tokens->tokens[tokens_s].type = type;
  tokens->tokens[tokens_s].string = st_new_subc(p, p_s, st_enc_utf8);
  tokens->tokens[tokens_s].start.line = ltoken_line;
  tokens->tokens[tokens_s].start.column = ltoken_column;

  tokens->tokens[tokens_s].end.line = line;
  tokens->tokens[tokens_s].end.column = column;
  ++tokens->size;

  FL_DBG("[%zu:%zu] fl_tokenize_push(%s)\n", line, column,
         tokens->tokens[tokens_s].string->value);
}

void fl_tokenize_flush(fl_token_list_t* tokens, fl_tokens_t type,
                       fl_tk_state_t* lstate, fl_tk_state_t* state) {
  size_t tokens_s = tokens->size;

  tokens->tokens[tokens_s].type = type;
  tokens->tokens[tokens_s].string =
      st_new_subc(lstate->itr, state->itr - lstate->itr, st_enc_utf8);
  tokens->tokens[tokens_s].start.line = lstate->line;
  tokens->tokens[tokens_s].start.column = lstate->column;

  tokens->tokens[tokens_s].end.line = state->line;
  tokens->tokens[tokens_s].end.column = state->column;
  ++tokens->size;

  FL_DBG("[%zu:%zu] fl_tokenize_flush(%s)\n", lstate->line, lstate->column,
         tokens->tokens[tokens_s].string->value);

  fl_tokenize_cp_state(state, lstate);
}

void fl_token_process(fl_token_list_t* tokens, fl_tokens_cfg_t* tk,
                      fl_tk_state_t* state, fl_tk_state_t* lstate) {
  size_t size = (state->itr - lstate->itr);
  if (size) {
    fl_tokenize_flush(tokens, FL_TK_UNKOWN, lstate, state);
  }

  if (tk->type == FL_TK_NEWLINE) {
    ++state->line;
    state->column = 1;
  } else {
    state->column += tk->text_s;
  }

  // add a fake new line at the end, will help the parser
  if (tk->type == FL_TK_EOF) {
    fl_tokenize_push(tokens, FL_TK_NEWLINE, lstate->itr, tk->text_s,
                     lstate->line, lstate->column, state->line, state->column);
  }

  state->itr += tk->text_s;
  fl_tokenize_flush(tokens, tk->type, lstate, state);
}

void fl_tokenize_cp_state(fl_tk_state_t* src, fl_tk_state_t* dst) {
  dst->itr = src->itr;
  dst->line = src->line;
  dst->column = src->column;
}

void fl_tokens_debug(fl_token_list_t* tokens) {
  size_t i = 0;
  for (; i < tokens->size; ++i) {
    fl_token_t* token = &tokens->tokens[i];
    // print debug tokens
    dbg_verbose("%6zu|%3d[%3zu:%3zu - %3zu:%3zu] %s\n", i, token->type,
                token->start.line, token->start.column, token->end.line,
                token->end.column, token->string->value);
  }
}

fl_token_list_t* fl_tokenize(string* file) {
  // TODO resize!
  fl_token_list_t* tokens = (fl_token_list_t*)malloc(sizeof(fl_token_list_t) +
                                                     sizeof(fl_token_t) * 750);

  tokens->size = 0;

  char* start = file->value;
  char* tend;
  st_enc_t enc = file->encoding;
  st_len_t jump;
  fl_tokens_cfg_t* tk = 0;

  size_t tokens_size = 0;
  size_t zone = 0;

  fl_tk_state_t state;
  state.line = 1;
  state.column = 1;
  state.itr = start;
  state.end = start + file->used + 1; // include zeronull

  fl_tk_state_t lstate;
  lstate.line = 1;
  lstate.column = 1;
  lstate.itr = start;

  char* last_space = 0;

  while (state.itr < state.end) {
    // FL_DBG("[%p] - [%p]\n", state.itr, state.end);
    // FL_DBG("[%c]\n", *(state.itr));

    FL_DBG("[%zu:%zu] read [%c]\n", state.line, state.column, *state.itr);
    // push spaces as independent tokens
    if (!last_space && *(state.itr) == ' ') {
      FL_DBG("[%zu:%zu] space start\n", state.line, state.column);
      if (state.itr != lstate.itr) {
        fl_tokenize_flush(tokens, FL_TK_UNKOWN, &lstate, &state);
      }

      last_space = state.itr;
    }
    if (last_space && *(state.itr) != ' ') {
      FL_DBG("[%zu:%zu] space end\n", state.line, state.column);
      fl_tokenize_flush(tokens, FL_TK_WHITESPACE, &lstate, &state);
      last_space = 0;
    }

    // TODO optimize, split tokens + strings
    size_t diff = state.end - state.itr;
    tk = fl_get_token(state.itr, diff + 1);
    // check next char, allow 'for*'
    if (tk && !tk->is_punctuation) {
      if (diff > tk->text_s &&
          (isdigit(state.itr[tk->text_s]) || isalpha(state.itr[tk->text_s]) ||
           state.itr[tk->text_s] == '_')) {
        tk = 0; // invalid, is part of a bigger word
      }
    }

    if (tk) {
      // check that token position valid
      if (isalpha(*state.itr)) {
        // if current is alpha, previous must not be.
        if (isalpha(*(state.itr - 1))) {
          tk = 0; // like a not found token
        }
      }
    }

    if (tk) {
      fl_token_process(tokens, tk, &state, &lstate);

      // continue until close_text
      if (tk->close_text) {
        tend = state.end - tk->close_text_s;
        while (state.itr < tend) {
          if (memcmp(tk->close_text, state.itr, tk->close_text_s) == 0) {
            if (tk->escapable) {
              if (fl_get_escape_count(state.itr, start) % 2 == 0) {
                fl_token_process(tokens, tk, &state, &lstate);
                break;
              }
            } else {
              break;
            }
          }
          ++state.itr;
        }
      }
    } else {
      ++state.column;
      jump = st_char_size(state.itr, enc);
      state.itr += jump;
    }
  }

  fl_tokens_debug(tokens);
#ifdef FL_VERBOSE
#endif

  return tokens;
}

void fl_tokens_delete(fl_token_list_t* tokens) {
  size_t i = 0;
  for (; i < tokens->size; ++i) {
    st_delete(&tokens->tokens[i].string);
  }
  free(tokens);
}

#undef FL_DBG
