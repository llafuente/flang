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

PSR_READ_IMPL(literal) {
  fl_ast_t* ast;

  FL_TRY_READ(lit_null);
  FL_TRY_READ(lit_boolean);
  FL_TRY_READ(lit_string);
  FL_TRY_READ(lit_numeric);
  FL_TRY_READ(lit_identifier);

  return 0;
}

PSR_READ_IMPL(lit_null) {
  PSR_AST_START(FL_AST_LIT_NULL);

  if (PSR_ACCEPT("null") || PSR_ACCEPT("nil")) {
    PSR_AST_RET();
  }
  PSR_AST_RET_NULL();
}

PSR_READ_IMPL(lit_boolean) {
  PSR_AST_START(FL_AST_LIT_BOOLEAN);

  if (PSR_ACCEPT_TOKEN(FL_TK_TRUE)) {
    ast->boolean.value = true;
    PSR_AST_RET();
  }
  if (PSR_ACCEPT_TOKEN(FL_TK_FALSE)) {
    ast->boolean.value = false;
    PSR_AST_RET();
  }

  PSR_AST_RET_NULL();
}

PSR_READ_IMPL(lit_string_sq) {
  PSR_AST_START(FL_AST_LIT_STRING);

  if (!PSR_ACCEPT_TOKEN(FL_TK_SQUOTE)) {
    PSR_AST_RET_NULL();
  }
  ast->string.value = state->token->string;
  PSR_NEXT();

  if (!PSR_ACCEPT_TOKEN(FL_TK_SQUOTE)) {
    PSR_AST_RET_NULL();
  }

  PSR_AST_RET();
}

PSR_READ_IMPL(lit_string_dq) {
  PSR_AST_START(FL_AST_LIT_STRING);

  if (!PSR_ACCEPT_TOKEN(FL_TK_DQUOTE)) {
    PSR_AST_RET_NULL();
  }
  ast->string.value = state->token->string;
  PSR_NEXT();

  if (!PSR_ACCEPT_TOKEN(FL_TK_DQUOTE)) {
    PSR_AST_RET_NULL();
  }

  PSR_AST_RET();
}

PSR_READ_IMPL(lit_string) {
  fl_ast_t* ast;

  ast = PSR_READ(lit_string_sq);

  if (ast) {
    return ast;
  }

  ast = PSR_READ(lit_string_dq);
  if (ast) {
    return ast;
  }

  return 0;
}

// TODO
PSR_READ_IMPL(lit_array) { return 0; }
// TODO
PSR_READ_IMPL(lit_object) { return 0; }

// TODO manage overflow/underflow errors
PSR_READ_IMPL(lit_numeric) {
  PSR_AST_START(FL_AST_LIT_NUMERIC);

  // null is and alias of 0

  if (PSR_ACCEPT_TOKEN(FL_TK_NULL)) {
    ast->numeric.value = 0;
    PSR_NEXT();
    PSR_AST_RET();
  }

  string* str = state->token->string;
  char* start = str->value;
  if (isdigit(start[0])) {
    char* endp = start + (str->used);
    // must be a number and error will be final!
    double result = strtod(start, &endp);
    if (errno) {
      if ((result == HUGE_VAL || result == -HUGE_VAL) && errno == ERANGE) {
        fprintf(stderr, "ERROR! overflow\n");
      } else if (errno == ERANGE) {
        fprintf(stderr, "ERROR! underflow\n");
      }
    }
    PSR_NEXT();

    ast->numeric.value = result;
    PSR_AST_RET();
  }

  PSR_AST_RET_NULL();
}

// TODO review what should be valid and what not
// right now we should accept "anything that is not token"
PSR_READ_IMPL(lit_identifier) {
  if (state->token->type == FL_TK_UNKOWN) {
    PSR_AST_START(FL_AST_LIT_IDENTIFIER);

    ast->identifier.string = st_clone(state->token->string);
    PSR_NEXT();

    PSR_AST_RET();
  }

  return 0;
}
