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
TASK_IMPL(parser_functions) {
  fl_ast_t* root;
  fl_ast_t** body;
  fl_ast_t* err;

  root = fl_parse_utf8("fn x() {}");
  CHK_GET_BODY(root, body);
  ASSERT(body[0]->type == FL_AST_DECL_FUNCTION, "FL_AST_DECL_FUNCTION");
  ASSERT(body[0]->func.id->type == FL_AST_LIT_IDENTIFIER,
         "FL_AST_LIT_IDENTIFIER");
  ASSERT(body[0]->func.params == 0, "no args");
  fl_ast_delete(root);

  root = fl_parse_utf8("fn x(yy, zz , mm ,xx) {}");
  CHK_GET_BODY(root, body);
  ASSERT(body[0]->type == FL_AST_DECL_FUNCTION, "FL_AST_DECL_FUNCTION");
  ASSERT(body[0]->func.id->type == FL_AST_LIT_IDENTIFIER,
         "FL_AST_LIT_IDENTIFIER");
  ASSERT(body[0]->func.params != 0, "no args");
  fl_ast_delete(root);

  root = fl_parse_utf8("fn {}");
  CHK_ERROR(root, err, "cannot parse function identifier");
  CHK_ERROR_RANGE(err, 4, 1, 5, 1);
  fl_ast_delete(root);

  root = fl_parse_utf8("fn hell ({}");
  CHK_ERROR(root, err, "expected identifier");
  fl_ast_delete(root);

  root = fl_parse_utf8("fn x a");
  CHK_ERROR(root, err, "expected '('");
  CHK_ERROR_RANGE(err, 6, 1, 7, 1);
  fl_ast_delete(root);

  root = fl_parse_utf8("fn (){};");
  CHK_ERROR(root, err, "cannot parse function identifier");
  CHK_ERROR_RANGE(err, 4, 1, 5, 1);
  fl_ast_delete(root);

  root = fl_parse_utf8("fn x () { fn (){}; }");
  CHK_ERROR(root, err, "cannot parse function identifier");
  CHK_ERROR_RANGE(err, 14, 1, 15, 1);
  fl_ast_delete(root);

  // TODO 'template'
  root = fl_parse_utf8("fn x(i8 arg1, i8 arg2) { return arg1 + arg2;}");
  CHK_BODY(root);
  fl_ast_delete(root);

  // declaration only
  root = fl_parse_utf8(
      "fn x( i8 arg1 , i8 arg2 ) : i8 ; fn printf2( ptr<i8> format, ... ) ;");
  CHK_BODY(root);
  fl_codegen(root, "test");
  fl_ast_delete(root);
  return 0;
}
