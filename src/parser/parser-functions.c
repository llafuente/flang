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
#include "grammar/tokens.h"
#include "grammar/parser.h"

ast_t* fl_parse(string* code) {
  ast_t* root;
  yy_scan_string(code->value);
  yyparse(&root);

  return root;
}
// TODO parse core!
ast_t* fl_parse_utf8(char* str) {
  st_size_t cap;
  size_t len = st_utf8_length(str, &cap);

  string* code = st_new(cap + 2, st_enc_utf8);
  st_copyc(&code, str, st_enc_utf8);
  st_append_char(&code, 0);

  return fl_parse(code);
}

ast_t* fl_parse_file(const char* filename) {
  string* code = fl_file_to_string(filename);
  return fl_parse(code);
}

string* fl_file_to_string(const char* filename) {
  FILE* f = fopen(filename, "r");
  if (!f) {
    fprintf(stderr, "file cannot be opened: %s\n", filename);
    exit(3);
  }

  fseek(f, 0, SEEK_END);
  size_t lSize = ftell(f);
  rewind(f);

  string* code = st_new(lSize + 2, st_enc_utf8);
  // copy the file into the buffer:
  size_t result = fread(code->value, 1, lSize, f);
  if (result != lSize) {
    fprintf(stderr, "Reading error\n");
    exit(3);
  }

  code->used = result;
  code->length = st_utf8_length(code->value, 0);
  st__zeronull(code->value, result, st_enc_utf8);
  st_append_char(&code, 0); // \0\0 EOF

  return code;
}

ast_t* fl_parse_main_file(const char* filename) {
  string* code = fl_file_to_string(filename);

  ast_t* import = ast_mk_import(ast_mk_lit_string("\"core/core\"", false));

  ast_t* root = fl_parse(code);

  assert(root->type == FL_AST_PROGRAM);
  ast_t* block = root->program.body;
  if (block->type != FL_AST_ERROR) {
    ast_mk_list_insert(block->block.body, import, 0);
  }

  return root;
}

ast_t* fl_passes(ast_t* root) {
  psr_load_imports(root);

  ts_register_types(root);
  // do inference
  root = ts_pass(root);
  ast_dump(root);

  return root;
}
