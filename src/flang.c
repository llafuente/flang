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

#include "flang/flang.h"
#include "flang/libparser.h"
#include "flang/debug.h"
#include "flang/libast.h"
#include "flang/libts.h"
#include "stringc.h"
#include <setjmp.h>

array* identifiers;
extern int yycolumn;
extern int yylineno;
jmp_buf fl_on_error_jmp;
FILE* ast_dump_file = 0;

void* pool_realloc_fix_string(void* ptr, size_t bytes) {
  string* str = (string*)ptr;
  string* ret = st_new(bytes, st_enc_utf8);
  st_copy(&ret, str);
  pool_free(str);

  return ret;
}

void flang_init() {
  memset(fl_on_error_jmp, 0, sizeof(jmp_buf));

  // reset flex/bison
  yycolumn = 1;
  yylineno = 1;

  // reset error state
  ast_last_error_message = 0;
  ast_last_error_node = 0;

  pool_init(2048);

  st_replace_allocators(pool_new, pool_realloc_fix_string, pool_free);
  hash_replace_allocators(pool_new, pool_realloc, pool_free);
  array_replace_allocators(pool_new, pool_realloc, pool_free);

  identifiers = malloc(sizeof(array));
  array_new(identifiers);

  ast_dump_file = stdout;

  ts_init();
}

void flang_exit(ast_t* root) {
  ast_delete(root);
  pool_destroy();

  ts_exit();
  array_delete(identifiers);
  free(identifiers);
  identifiers = 0;

  st_memfree();
  st_replace_allocators(malloc, realloc, free);
  hash_replace_allocators(malloc, realloc, free);
  array_replace_allocators(malloc, realloc, free);
}
