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

FL_READER_IMPL(literal) {
  fl_ast_t* ast;

  FL_TRY_READ(lit_null);
  FL_TRY_READ(lit_boolean);
  FL_TRY_READ(lit_string);
  FL_TRY_READ(lit_numeric);
  FL_TRY_READ(lit_identifier);

  return 0;
}

FL_READER_IMPL(lit_null) {
  FL_AST_START(FL_AST_LIT_NULL);

  if (FL_ACCEPT("null") || FL_ACCEPT("nil")) {
    FL_RETURN_AST();
  }
  FL_RETURN_NOT_FOUND();
}

FL_READER_IMPL(lit_boolean) {
  FL_AST_START(FL_AST_LIT_BOOLEAN);

  if (FL_ACCEPT_TOKEN(FL_TK_TRUE)) {
    ast->boolean.value = true;
    FL_RETURN_AST();
  }
  if (FL_ACCEPT_TOKEN(FL_TK_FALSE)) {
    ast->boolean.value = false;
    FL_RETURN_AST();
  }

  FL_RETURN_NOT_FOUND();
}

FL_READER_IMPL(lit_string_sq) {
  FL_AST_START(FL_AST_LIT_STRING);

  if (!FL_ACCEPT_TOKEN(FL_TK_SQUOTE)) {
    FL_RETURN_NOT_FOUND();
  }
  ast->string.value = state->token->string;
  FL_NEXT();

  if (!FL_ACCEPT_TOKEN(FL_TK_SQUOTE)) {
    FL_RETURN_NOT_FOUND();
  }

  return ast;
}

FL_READER_IMPL(lit_string_dq) {
  FL_AST_START(FL_AST_LIT_STRING);

  if (!FL_ACCEPT_TOKEN(FL_TK_DQUOTE)) {
    FL_RETURN_NOT_FOUND();
  }
  ast->string.value = state->token->string;
  FL_NEXT();

  if (!FL_ACCEPT_TOKEN(FL_TK_DQUOTE)) {
    FL_RETURN_NOT_FOUND();
  }

  return ast;
}

FL_READER_IMPL(lit_string) {
  fl_ast_t* ast;

  ast = FL_READ(lit_string_sq);

  if (ast) {
    return ast;
  }

  ast = FL_READ(lit_string_dq);
  if (ast) {
    return ast;
  }

  return 0;
}

// TODO
FL_READER_IMPL(lit_array) { return 0; }
// TODO
FL_READER_IMPL(lit_object) { return 0; }

FL_READER_IMPL(lit_numeric) {
  FL_AST_START(FL_AST_LIT_NUMERIC);

  printf("litnumeric\n");

  string* str = state->token->string;
  char* start = str->value;
  if (isdigit(start[0])) {
    char** end;
    *end = (start + str->used);
    // must be a number and error will be final!
    double result = strtod(start, end);
    if (errno) {
      if ((result == HUGE_VAL || result == -HUGE_VAL) && errno == ERANGE) {
        fprintf(stderr, "ERROR! overflow\n");
      } else if (errno == ERANGE) {
        fprintf(stderr, "ERROR! underflow\n");
      }
    }
    ast->numeric.value = result;
    printf("found\n");
    return ast;
  }

  printf("not-found\n");

  FL_RETURN_NOT_FOUND();
}

// TODO review what should be valid and what not
// right now we should accept "anything that is not token"
FL_READER_IMPL(lit_identifier) {
  if (state->token->type == FL_TK_UNKOWN) {
    FL_AST_START(FL_AST_LIT_IDENTIFIER);
    ast->identifier.string = st_clone(state->token->string);
    return ast;
  }

  return 0;
}
