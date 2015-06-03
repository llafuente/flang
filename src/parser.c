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
  fl_parser_init_types();

  fl_psrstack_t* stack = malloc(sizeof(fl_psrstack_t));
  fl_psrstate_t* state = malloc(sizeof(fl_psrstate_t));

  fl_parser_stack_init(stack, tokens, state);

  PSR_AST_START(FL_AST_PROGRAM);

  fl_ast_t* body;

  ast->program.tokens = tokens;
  ast->program.body = PSR_READ(program_block);

  PSR_AST_END();

  free(stack);
  free(state);

  return ast;
}

void fl_parse_core(fl_ast_t* ast) {
  ast->program.core = fl_parse_file("./../core/ffi-c.fl", true);
}

fl_ast_t* fl_parse(string* code) {
  fl_token_list_t* tokens;

  tokens = fl_tokenize(code);

  fl_ast_t* root = fl_parser(tokens);

  root->program.code = code;

  fl_ast_traverse(root, fl_ast_debug_cb, 0, 0);
#ifdef FL_VERBOSE
#endif

  return root;
}

fl_ast_t* fl_parse_utf8(char* str) {
  string* code;

  code = st_newc(str, st_enc_utf8);

  return fl_parse(code);
}

fl_ast_t* fl_parse_file(char* filename, bool core) {
  FILE* f = fopen(filename, "r");
  if (!f) {
    fprintf(stderr, "file cannot be opened: %s\n", filename);
    exit(3);
  }

  fseek(f, 0, SEEK_END);
  size_t lSize = ftell(f);
  rewind(f);

  string* code = st_new(lSize, st_enc_utf8);
  // copy the file into the buffer:
  size_t result = fread(code->value, 1, lSize, f);
  if (result != lSize) {
    fprintf(stderr, "Reading error\n");
    exit(3);
  }

  code->used = result;
  code->length = st_utf8_length(code->value, 0);
  st__zeronull(code->value, result, st_enc_utf8);

  fl_ast_t* r = fl_parse(code);

  if (!core) {
    // parse core
    fl_parse_core(r);
  }

  return r;
}
