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
#include "test.h"

// TODO review if ";" is required
TASK_IMPL(parser_literals) {
  fl_ast_t* root;
  fl_ast_t** body;

  root = fl_parse_utf8("\"hello:\\\"w\'orld\"");
  CHK_GET_BODY(root, body);
  ASSERT(body[0]->type == FL_AST_LIT_STRING, "FL_AST_LIT_STRING");
  fl_ast_delete(root);

  root = fl_parse_utf8("'hello:\"wo\\\'rld'");
  CHK_GET_BODY(root, body);
  ASSERT(body[0]->type == FL_AST_LIT_STRING, "FL_AST_LIT_STRING");
  fl_ast_delete(root);

  root = fl_parse_utf8("null");
  CHK_GET_BODY(root, body);
  ASSERT(body[0]->type == FL_AST_LIT_NULL, "FL_AST_LIT_NULL");
  fl_ast_delete(root);

  root = fl_parse_utf8("nil");
  CHK_GET_BODY(root, body);
  ASSERT(body[0]->type == FL_AST_LIT_NULL, "FL_AST_LIT_NULL");
  fl_ast_delete(root);

  root = fl_parse_utf8("true");
  CHK_GET_BODY(root, body);
  ASSERT(body[0]->type == FL_AST_LIT_BOOLEAN, "FL_AST_LIT_BOOLEAN");
  ASSERT(body[0]->boolean.value == true, "value = true");
  fl_ast_delete(root);

  root = fl_parse_utf8("false");
  CHK_GET_BODY(root, body);
  ASSERT(body[0]->type == FL_AST_LIT_BOOLEAN, "FL_AST_LIT_BOOLEAN");
  ASSERT(body[0]->boolean.value == false, "value = false");
  fl_ast_delete(root);

  root = fl_parse_utf8("1567");
  CHK_GET_BODY(root, body);
  ASSERT(body[0]->type == FL_AST_LIT_NUMERIC, "FL_AST_LIT_NUMERIC");
  ASSERT(body[0]->numeric.value == 1567, "value = 1567");
  fl_ast_delete(root);

  root = fl_parse_utf8("1e1");
  CHK_GET_BODY(root, body);
  ASSERT(body[0]->type == FL_AST_LIT_NUMERIC, "FL_AST_LIT_NUMERIC");
  ASSERT(body[0]->numeric.value == 10, "value = 10");
  fl_ast_delete(root);

  root = fl_parse_utf8("0xff");
  CHK_GET_BODY(root, body);
  ASSERT(body[0]->type == FL_AST_LIT_NUMERIC, "FL_AST_LIT_NUMERIC");
  ASSERT(body[0]->numeric.value == 0xff, "value = 0xff");
  fl_ast_delete(root);

  // TODO binary 0b000000001
  // TODO octal 0o777

  root = fl_parse_utf8("5.2");
  CHK_GET_BODY(root, body);
  ASSERT(body[0]->type == FL_AST_LIT_NUMERIC, "FL_AST_LIT_NUMERIC");
  ASSERT(body[0]->numeric.value == 5.2, "value = 5.2");
  fl_ast_delete(root);

  root = fl_parse_utf8("wtf");
  CHK_GET_BODY(root, body);
  ASSERT(body[0]->type == FL_AST_LIT_IDENTIFIER, "FL_AST_LIT_IDENTIFIER");
  ASSERT(strcmp(body[0]->identifier.string->value, "wtf") == 0,
         "identifier = wtf");
  st_delete(&body[0]->identifier.string);
  fl_ast_delete(root);

  return 0;
}
