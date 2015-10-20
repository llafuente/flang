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

ast_t* fl_parser(tk_token_list_t* tokens, bool attach_core) {
  ts_init();

  fl_psrstack_t* stack = malloc(sizeof(fl_psrstack_t));
  fl_psrstate_t* state = malloc(sizeof(fl_psrstate_t));

  psr_stack_init(stack, tokens, state);

  PSR_START(root, FL_AST_PROGRAM);

  root->program.tokens = tokens;

  // read core
  if (attach_core) {
    int olog_debug_level = log_debug_level;
    log_debug_level = 0; // remove to debug core parsing
    root->program.core = fl_parse_file("./../core/core.fl", false);
    log_debug_level = olog_debug_level;
  }

  root->program.body = PSR_READ(program_block);

  PSR_END(root);

  free(stack);
  free(state);

  return root;
}

ast_t* fl_parse(string* code, bool attach_core) {
  tk_token_list_t* tokens;

  tokens = fl_tokenize(code);

  ast_t* root = fl_parser(tokens, attach_core);
  root->program.code = code;

  // do inference
  ast_parent(root); // set node->parent

  ast_dump(root);
  root = ts_pass(root);
  // TODO remove this, just for debugging purpose
  ast_dump(root);

  return root;
}

ast_t* fl_parse_utf8(char* str) {
  string* code;

  code = st_newc(str, st_enc_utf8);

  return fl_parse(code, true);
}

ast_t* fl_parse_file(const char* filename, bool attach_core) {
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

  ast_t* r = fl_parse(code, attach_core);

  return r;
}
