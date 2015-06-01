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

string* file_to_string(char* filename) {
  FILE* f = fopen(filename, "r");
  if (!f) {
    fprintf(stderr, "file cannot be opened: %s\n", filename);
    exit(3);
  }

  fseek(f, 0, SEEK_END);
  size_t lSize = ftell(f);
  rewind(f);

  string* file = st_new(lSize, st_enc_utf8);
  // copy the file into the buffer:
  size_t result = fread(file->value, 1, lSize, f);
  if (result != lSize) {
    fprintf(stderr, "Reading error\n");
    exit(3);
  }

  file->used = result;
  file->length = st_utf8_length(file->value, 0);
  st__zeronull(file->value, result, st_enc_utf8);

  return file;
}
// _popen and _pclose for Windows.
// 2>&1
string* execute(char* cmd) {
  printf("execute: %s\n", cmd);

  FILE* pipe = popen(cmd, "r");
  if (!pipe) {
    fprintf(stderr, "cannot execute: %s", cmd);
    exit(1);
  }

  string* output = st_new(1024, st_enc_utf8);

  char* pos = output->value;

  while (!feof(pipe)) {
    if (fgets(pos, 1024, pipe) != NULL) {
      pos = pos + strlen(pos);
    }
  }
  pclose(pipe);

  if (output->value[0] == '\0') {
    fprintf(stderr, "no output for: %s", cmd);
    exit(1);
  }

  return output;
}

TASK_IMPL(flang_files) {

  char* files[] = {"../test/fl/expressions"};
  char* fl_file = malloc(sizeof(char) * 100);
  char* txt_file = malloc(sizeof(char) * 100);
  char* bc_file = malloc(sizeof(char) * 100);
  char* ir_file = malloc(sizeof(char) * 100);
  char* cmd = malloc(sizeof(char) * 150);

  fl_ast_t* root;
  LLVMModuleRef module;
  string* code;
  size_t i;

  for (i = 0; i < 2; i += 2) {
    fl_file[0] = '\0';
    strcat(fl_file, files[i]);
    strcat(fl_file, ".fl");
    txt_file[0] = '\0';
    strcat(txt_file, files[i]);
    strcat(txt_file, ".txt");
    bc_file[0] = '\0';
    strcat(bc_file, files[i]);
    strcat(bc_file, ".bc");
    ir_file[0] = '\0';
    strcat(ir_file, files[i]);
    strcat(ir_file, ".ir");

    code = file_to_string(fl_file);

    root = fl_parse(code);

    module = fl_codegen(root, "test");

    fl_to_bitcode(module, bc_file);
    fl_to_ir(module, ir_file);

    fl_ast_delete(root);
    cmd[0] = '\0';
    strcat(cmd, "lli ");
    strcat(cmd, ir_file);
    strcat(cmd, " 2>&1");
    string* output = execute(cmd);
    string* output_cmp = file_to_string(txt_file);

    // printf("output \n--\n%s\n--\n", output->value);
    // printf("output-cmd \n--\n%s\n--\n", output_cmp->value);

    if (strcmp(output->value, output_cmp->value) != 0) {
      // save output to diff!

      ASSERT(false, files[i]);
      assert(false);
    }
    st_delete(&output);
    st_delete(&output_cmp);
  }

  free(fl_file);
  free(txt_file);
  free(bc_file);
  free(ir_file);
  free(cmd);

  return 0;
}