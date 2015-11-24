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

ast_t* fl_attach_core(ast_t* root) {
  ast_t* block = root->program.body;

  if (block->type != FL_AST_ERROR) {
    ast_t* import = ast_mk_import(ast_mk_lit_string("core/core", false));
    ast_mk_list_insert(block->block.body, import, 0);
  }
}

ast_t* fl_parse(string* code, const char* file) {
  // create program node, so error reporting could be nice!
  ast_t* root = ast_mk_program(0);
  root->program.code = code;
  root->program.file = file ? strdup(file) : 0;

  YY_BUFFER_STATE buf = yy_scan_string(code->value);
  yyparse(&root);
  yy_delete_buffer(buf);

  return root;
}
// TODO parse core!
ast_t* fl_parse_utf8(char* str) {
  st_size_t cap;
  size_t len = st_utf8_length(str, &cap);

  string* code = st_new(cap + 2, st_enc_utf8);
  st_copyc(&code, str, st_enc_utf8);
  st_append_char(&code, 0);

  return fl_parse(code, 0);
}

ast_t* fl_parse_main_utf8(char* str) {
  ast_t* root = fl_parse_utf8(str);
  fl_attach_core(root);

  return root;
}

ast_t* fl_parse_file(const char* filename) {
  string* code = fl_file_to_string(filename);
  return fl_parse(code, filename);
}

string* fl_file_to_string(const char* filename) {
  FILE* f = fopen(filename, "r");
  if (!f) {
    fl_fatal_error("file cannot be opened: %s\n", filename);
  }

  fseek(f, 0, SEEK_END);
  size_t lSize = ftell(f);
  rewind(f);

  string* code = st_new(lSize + 2, st_enc_utf8);
  // copy the file into the buffer:
  size_t result = fread(code->value, 1, lSize, f);
  if (result != lSize) {
    fl_fatal_error(stderr, "Reading error\n");
  }

  // double null needed by flex/bison
  code->value[lSize] = '\0';
  code->value[lSize + 1] = '\0';

  code->used = result;
  code->length = st_utf8_length(code->value, 0);

  return code;
}

ast_t* fl_parse_main_file(const char* filename) {
  string* code = fl_file_to_string(filename);

  ast_t* root = fl_parse(code, filename);

  assert(root->type == FL_AST_PROGRAM);
  fl_attach_core(root);

  return root;
}

ast_t* typesystem(ast_t* root) {
  psr_load_imports(root);

  ts_register_types(root);

  // do inference
  root = ts_pass(root);

  return root;
}
