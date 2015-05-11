#include "flang.h"

struct tokenize_state {
  size_t line;
  size_t column;
  char* itr;
  char* end;
};

typedef struct tokenize_state tokenize_state_t;

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

void fl_tokenize_push(fl_token_list_t* tokens, char* p, size_t p_s,
                      size_t ltoken_line, size_t ltoken_column, size_t line,
                      size_t column) {
  size_t tokens_s = tokens->size;
  tokens->tokens[tokens_s].value = st_new_subc(p, p_s, st_enc_utf8);
  tokens->tokens[tokens_s].start.line = ltoken_line;
  tokens->tokens[tokens_s].start.column = ltoken_column;

  tokens->tokens[tokens_s].end.line = line;
  tokens->tokens[tokens_s].end.column = column;
  ++tokens->size;
}

void fl_token_process(fl_token_list_t* tokens, fl_tokens_cfg_t* tk,
                      tokenize_state_t* state, tokenize_state_t* lstate) {
  size_t size = (state->itr - lstate->itr);
  if (size) {
    fl_tokenize_push(tokens, lstate->itr, size, lstate->line, lstate->column,
                     state->line, state->column);

    lstate->itr = state->itr;
    lstate->line = state->line;
    lstate->column = state->column;
  }

  if (tk->token != FL_TK_NEWLINE) {
    ++state->line;
    state->column = 1;
  } else {
    state->column += tk->text_s;
  }

  fl_tokenize_push(tokens, lstate->itr, tk->text_s, lstate->line,
                   lstate->column, state->line, state->column);

  state->itr += tk->text_s;
  lstate->itr = state->itr;
  lstate->line = state->line;
  lstate->column = state->column;
}

void fl_tokens_debug(fl_token_t* tokens, size_t tokens_s) {
  size_t i = 0;
  for (; i < tokens_s; ++i) {
    // print debug tokens
    printf("[%zu|%zu:%zu-%zu:%zu] %s\n", i, tokens[i].start.line,
           tokens[i].start.column, tokens[i].end.line, tokens[i].end.column,
           tokens[i].value->value);
  }
}

fl_token_list_t* fl_tokenize(string* file) {
  // TODO resize!
  fl_token_list_t* tokens = (fl_token_list_t*)malloc(sizeof(fl_token_list_t) +
                                                     sizeof(fl_token_t) * 500);

  tokens->size = 0;

  char* start = file->value;
  char* tend;
  st_enc_t enc = file->encoding;
  st_len_t jump;
  fl_tokens_cfg_t* tk = 0;

  size_t tokens_size = 0;
  size_t zone = 0;

  tokenize_state_t state;
  state.line = 1;
  state.column = 1;
  state.itr = start;
  state.end = start + file->used + 1; // include zeronull

  tokenize_state_t lstate;
  lstate.line = 1;
  lstate.column = 1;
  lstate.itr = start;

  while (state.itr < state.end) {
    tk = fl_get_token(state.itr, state.end - state.itr);
    if (tk != FL_TK_UNKOWN) {
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

  return tokens;
}


void fl_tokens_delete(fl_token_list_t* tokens) {
  size_t i = 0;
  for (; i < tokens->size; ++i) {
    st_delete(&tokens->tokens[i].value);
  }
  free(tokens);
}
