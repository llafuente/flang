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
*/
PSR_READ_IMPL(type) {
  PSR_AST_START(FL_AST_TYPE);

  // primitives
  fl_tokens_t tks[] = {FL_TK_STRING, FL_TK_F64, FL_TK_F32, FL_TK_U64,
                       FL_TK_U32,    FL_TK_U16, FL_TK_U8,  FL_TK_I64,
                       FL_TK_I32,    FL_TK_I16, FL_TK_I8,  FL_TK_BOOL};
  if (!fl_parser_accept_token_list(tokens, state, tks, 12)) {
    PSR_AST_RET_NULL();
  }

  ast->idtype.of = state->prev_token->type;

  PSR_AST_RET();
}
