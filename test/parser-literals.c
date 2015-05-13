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
// TODO review if ";" is required
TASK_IMPL(parser_literals) {
  fl_ast_t* ast;

  ast = fl_parse_utf8("\"hello:\\\"w\'orld\"");
  ASSERT(ast != 0, "string literal found!");
  ASSERT(ast->type == FL_AST_LIT_STRING, "FL_AST_LIT_STRING");
  free(ast);

  ast = fl_parse_utf8("'hello:\"wo\\\'rld'");
  ASSERT(ast != 0, "string literal found!");
  ASSERT(ast->type == FL_AST_LIT_STRING, "FL_AST_LIT_STRING");
  free(ast);

  ast = fl_parse_utf8("null");
  ASSERT(ast != 0, "null literal found!");
  ASSERT(ast->type == FL_AST_LIT_NULL, "FL_AST_LIT_NULL");
  free(ast);

  ast = fl_parse_utf8("nil");
  ASSERT(ast != 0, "null literal found!");
  ASSERT(ast->type == FL_AST_LIT_NULL, "FL_AST_LIT_NULL");
  free(ast);

  ast = fl_parse_utf8("true");
  ASSERT(ast != 0, "boolean literal found!");
  ASSERT(ast->type == FL_AST_LIT_BOOLEAN, "FL_AST_LIT_BOOLEAN");
  ASSERT(ast->boolean.value == true, "value = true");
  free(ast);

  ast = fl_parse_utf8("false");
  ASSERT(ast != 0, "boolean literal found!");
  ASSERT(ast->type == FL_AST_LIT_BOOLEAN, "FL_AST_LIT_BOOLEAN");
  ASSERT(ast->boolean.value == false, "value = true");
  free(ast);

#define FL_VERBOSE

  ast = fl_parse_utf8("1567");
  ASSERT(ast != 0, "numeric literal found!");
  ASSERT(ast->type == FL_AST_LIT_NUMERIC, "FL_AST_LIT_NUMERIC");
  ASSERT(ast->numeric.value == 1567, "value = true");
  free(ast);

  ast = fl_parse_utf8("1e1");
  ASSERT(ast != 0, "numeric literal found!");
  ASSERT(ast->type == FL_AST_LIT_NUMERIC, "FL_AST_LIT_NUMERIC");
  ASSERT(ast->numeric.value == 10, "value = true");
  free(ast);

  ast = fl_parse_utf8("0xff");
  ASSERT(ast != 0, "numeric literal found!");
  ASSERT(ast->type == FL_AST_LIT_NUMERIC, "FL_AST_LIT_NUMERIC");
  ASSERT(ast->numeric.value == 0xff, "value = true");
  free(ast);

  // TODO binary 0b000000001
  // TODO octal 0o777

  return 0;
}
