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

TASK_IMPL(tokenizer) {
  string* code;
  fl_token_list_t* tokens;
  // tets priority <= gt than '<' '='
  code = st_newc("a<=b;", st_enc_utf8);
  tokens = fl_tokenize(code);

  ASSERT(tokens->size == 6, "priority token test");

  fl_tokens_delete(tokens);
  st_delete(&code);

  code = st_newc("-2", st_enc_utf8);
  tokens = fl_tokenize(code);

  ASSERT(tokens->size == 4, "priority token test");

  fl_tokens_delete(tokens);
  st_delete(&code);


  // test escape string
  code = st_newc("log \"hello\"; log \"\\\"hell\\\"\"; ", st_enc_utf8);
  tokens = fl_tokenize(code);

  ASSERT(tokens->size == 16, "escape string test");

  fl_tokens_delete(tokens);
  st_delete(&code);

  // fl_tokens_debug(tokens);

  code = st_newc("1567", st_enc_utf8);
  tokens = fl_tokenize(code);

  ASSERT(tokens->size == 3, "just a number");

  fl_tokens_delete(tokens);
  st_delete(&code);

  code = st_newc("var x;", st_enc_utf8);
  tokens = fl_tokenize(code);

  ASSERT(tokens->size == 6, "var space x semicolon new-line eof");

  fl_tokens_delete(tokens);
  st_delete(&code);

  return 0;
}
