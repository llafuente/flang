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

#include "../tasks.h"
#include "flang/common.h"
#include "flang/libast.h"
#include "flang/debug.h"
#include "flang/flang.h"
#include "flang/libparser.h"
#include "flang/./libcg.h"
#include "flang/libts.h"

// _popen and _pclose for Windows.
// 2>&1
string* execute(char* cmd) {
  printf("# execute: %s\n", cmd);

  FILE* pipe = popen(cmd, "r");
  if (!pipe) {
    fprintf(stderr, "cannot execute: %s", cmd);
    exit(1);
  }

  string* output = st_new(4096, st_enc_utf8);

  char* pos = output->value;

  while (!feof(pipe)) {
    if (fgets(pos, 4096, pipe) != NULL) {
      pos = pos + strlen(pos);
    }
  }
  pclose(pipe);

  return output;
}

void test_file_list(char** files, size_t nfiles, char* path) {
  char* fl_file = malloc(sizeof(char) * 100);
  char* txt_file = malloc(sizeof(char) * 100);

  ast_t* root;
  size_t i;

  for (i = 0; i < nfiles; ++i) {
    printf("# file to test %zu '%s'\n", i, files[i]); // "clear screen"
    if (log_debug_level >= 4) {
      printf("\033[2J"); // "clear screen"
    }

    if (i == 11) {
      // log_debug_level = 10;
    }

    fl_file[0] = '\0';
    strcat(fl_file, path);
    strcat(fl_file, files[i]);
    strcat(fl_file, ".fl");
    txt_file[0] = '\0';
    strcat(txt_file, path);
    strcat(txt_file, files[i]);
    strcat(txt_file, ".txt");

    flang_init();
    root = psr_file_main(fl_file);

    if (ast_print_error(root)) {
      exit(3);
    }

    root = typesystem(root);
    if (ast_print_error(root)) {
      exit(4);
    }

    if (ast_last_error_message) {
      fprintf(stderr, "unexpected typesystem error");
      exit(1);
    }

    // ty_dump_table();

    fl_codegen(root);

    flang_exit(root);

    // compile
    string* output = execute(
        "clang -std=c11 -Wno-parentheses-equality -lpthread -luv -lstringc "
        "-D_GNU_SOURCE ./codegen/run.c -o ./codegen/app");
    st_delete(&output);

    // execute
    output = execute("././codegen/app");
    if (output->value[0] == '\0') {
      fprintf(stderr,
              "Test executed but no output. At least one line is required");
      exit(1);
    }
    string* output_cmp = psr_file_to_string(txt_file);

    // printf("output-cmd \n--\n%s\n--\n", output_cmp->value);

    if (strcmp(output->value, output_cmp->value) != 0) {
      // save output to diff!
      fprintf(stderr,
              "output of: %s\n--\n%s\n--\nexpected output \n--\n%s\n--\n",
              fl_file, output->value, output_cmp->value);
      exit(1);
    }
    st_delete(&output);
    st_delete(&output_cmp);
    // debug: if (i==1) exit(1);
  }

  free(fl_file);
  free(txt_file);
}

TASK_IMPL(flang_files) {
  printf("# flang_files\n");

  // find ./codegen/ | grep 'fl' | sed 's/\.fl//' | awk '{print "\"" $0 "\","}'

  char* test_files[] = {
      "typesystem/autocast",                "typesystem/type-promotion-mix",
      "typesystem/type-promotion-unsigned", "typesystem/types",
      "typesystem/casting",                 "typesystem/expressions",
      "typesystem/type-promotion-signed",   "statements/loops",
      "statements/if",                      "statements/loops2",
      "math/arithmetic",                    "math/increment",
      "memory/pointers2",                   "memory/pointer-math",
      "memory/memory3",                     "memory/pointers",
      "memory/memory",                      "misc/globals",
      "misc/log",                           "misc/globals2",
      "misc/fibonacci",
      //"misc/string",
      //"misc/hello-world",
      "functions/templates",                "functions/functions",
      "functions/function-pointer",
  };

  test_file_list(test_files, 26, "./test/codegen/");

  char* perf_files[] = {"array-reverse"};

  test_file_list(perf_files, 1, "test/perf/");

  return 0;
}
