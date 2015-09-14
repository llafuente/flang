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

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stringc.h>

#include "flang.h"

int main(int argc, const char* argv[]) {
  if (argc == 2) {
    printf("Usage: flan file.fl output.ir\n");
    exit(1);
  }

  FILE* f = fopen(argv[1], "r");
  if (!f) {
    fprintf(stderr, "file cannot be opened: %s\n", argv[1]);
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

  printf("read\n%s\n\ntokenize:\n", file->value);

  tk_token_list_t* tokens = fl_tokenize(file);

  ast_t* root = fl_parser(tokens, false);
  ast_dump(root);

  LLVMModuleRef module = fl_codegen(root, "test");

  fl_to_ir(module, argv[2]);

  ast_delete(root);

  fclose(f);
  st_delete(&file);

  return 0;
}
