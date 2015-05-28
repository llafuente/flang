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

/*
literal
  []
  {}
  text

format
  (literal)[]      // array
  (literal){}      // object
  *(literal)       // raw pointer
  ref<(literal)>   // wrapper

  void - LLVMVoidTypeInContext(state.context)

*/
PSR_READ_IMPL(type) {
  PSR_AST_START(FL_AST_TYPE);

  // primitives
  fl_tokens_t tk = state->token->type;
  fl_tokens_t tks[] = {FL_TK_BOOL,

                       FL_TK_I8,     FL_TK_U8,  FL_TK_I16, FL_TK_U16,
                       FL_TK_I32,    FL_TK_U32, FL_TK_I64, FL_TK_U64,

                       FL_TK_F32,    FL_TK_F64,

                       FL_TK_STRING, FL_TK_VOID};

  size_t i;
  for (i = 0; i < 13; ++i) {
    if (tk == tks[i]) {
      ast->ty.id = i;
      PSR_NEXT();

      PSR_AST_RET();
    }
  }

  PSR_AST_RET_NULL();
}

fl_type_t* fl_type_table = 0;

void fl_parser_init_types() {
  if (!fl_type_table) {
    printf("\n\n\n*****INIT TABLE****\n\n\n");
    fl_type_table = calloc(sizeof(fl_type_t), 100);
    // [0] bool
    fl_type_table[0].of = FL_NUMBER;
    fl_type_table[0].number.bits = 1;
    fl_type_table[0].number.fp = false;
    fl_type_table[0].number.sign = false;
    // [1-8] i8,u8,i16,u16,i32,u32,i64,u64
    size_t i = 1;
    size_t id = 1;
    for (; i < 5; i++) {
      fl_type_table[id].of = FL_NUMBER;
      fl_type_table[id].number.bits = i * 8;
      fl_type_table[id].number.fp = false;
      fl_type_table[id].number.sign = false;
      ++id;
      fl_type_table[id].of = FL_NUMBER;
      fl_type_table[id].number.bits = i * 8;
      fl_type_table[id].number.fp = false;
      fl_type_table[id].number.sign = true;
      ++id;
    }

    // [9] f32
    fl_type_table[id].of = FL_NUMBER;
    fl_type_table[id].number.bits = 32;
    fl_type_table[id].number.fp = true;
    fl_type_table[id].number.sign = true;
    ++id;

    // [10] f64
    fl_type_table[id].of = FL_NUMBER;
    fl_type_table[id].number.bits = 64;
    fl_type_table[id].number.fp = true;
    fl_type_table[id].number.sign = true;
    ++id;
  }
}
