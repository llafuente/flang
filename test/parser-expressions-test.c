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
TASK_IMPL(parser_expressions) {
  fl_ast_t* root;
  fl_ast_t* body;

  root = fl_parse_utf8("1+2");
  CHK_BODY(root, body);
  ASSERT(body->type == FL_AST_EXPR_BINOP, "FL_AST_EXPR_BINOP");
  ASSERT(body->binop.left->type == FL_AST_LIT_NUMERIC, "FL_AST_LIT_NUMERIC");
  ASSERT(body->binop.right->type == FL_AST_LIT_NUMERIC, "FL_AST_LIT_NUMERIC");
  fl_ast_delete(root);

  root = fl_parse_utf8("1+2+3");
  CHK_BODY(root, body);
  ASSERT(body->type == FL_AST_EXPR_BINOP, "FL_AST_EXPR_BINOP");
  ASSERT(body->binop.left->type == FL_AST_LIT_NUMERIC, "FL_AST_LIT_NUMERIC");
  ASSERT(body->binop.right->type == FL_AST_EXPR_BINOP, "FL_AST_EXPR_BINOP");
  ASSERT(body->binop.right->binop.left->type == FL_AST_LIT_NUMERIC,
         "FL_AST_LIT_NUMERIC");
  ASSERT(body->binop.right->binop.right->type == FL_AST_LIT_NUMERIC,
         "FL_AST_LIT_NUMERIC");
  fl_ast_delete(root);

  root = fl_parse_utf8("1*2");
  CHK_BODY(root, body);
  ASSERT(body->type == FL_AST_EXPR_BINOP, "FL_AST_EXPR_BINOP");
  ASSERT(body->binop.left->type == FL_AST_LIT_NUMERIC, "FL_AST_LIT_NUMERIC");
  ASSERT(body->binop.operator== FL_TK_ASTERISK, "FL_TK_ASTERISK");
  ASSERT(body->binop.right->type == FL_AST_LIT_NUMERIC, "FL_AST_LIT_NUMERIC");
  fl_ast_delete(root);

  root = fl_parse_utf8("1*2+3"); // priotity test
  CHK_BODY(root, body);
  ASSERT(body->type == FL_AST_EXPR_BINOP, "FL_AST_EXPR_BINOP");

  ASSERT(body->binop.left->binop.left->type == FL_AST_LIT_NUMERIC,
         "FL_AST_LIT_NUMERIC");
  ASSERT(body->binop.left->binop.operator== FL_TK_ASTERISK,
         "left FL_TK_ASTERISK");
  ASSERT(body->binop.left->binop.right->type == FL_AST_LIT_NUMERIC,
         "FL_AST_LIT_NUMERIC");

  ASSERT(body->binop.operator== FL_TK_PLUS, "FL_TK_PLUS");
  ASSERT(body->binop.right->type == FL_AST_LIT_NUMERIC, "FL_AST_LIT_NUMERIC");

  fl_ast_delete(root);

  root = fl_parse_utf8("-2"); // unary
  CHK_BODY(root, body);
  ASSERT(body->type == FL_AST_EXPR_LUNARY, "ast is FL_AST_EXPR_LUNARY");

  ASSERT(body->lunary.element->type == FL_AST_LIT_NUMERIC, "FL_AST_LIT_NUMERIC");
  ASSERT(body->lunary.operator== FL_TK_MINUS, "operator FL_TK_MINUS");

  fl_ast_delete(root);

  root = fl_parse_utf8("xxx++"); // unary
  CHK_BODY(root, body);
  ASSERT(body->type == FL_AST_EXPR_RUNARY, "ast is FL_AST_EXPR_RUNARY");

  ASSERT(body->lunary.element->type == FL_AST_LIT_IDENTIFIER,
         "FL_AST_LIT_IDENTIFIER");
  ASSERT(body->lunary.operator== FL_TK_PLUS2, "operator FL_TK_PLUS2");

  fl_ast_delete(root);

  // TODO nice hack for testing, but declaration first!
  root = fl_parse_utf8("a=b; var i8 a; var i8 b;"); // assignament
  CHK_BODY(root, body);
  ASSERT(body->type == FL_AST_EXPR_ASSIGNAMENT, "FL_AST_EXPR_ASSIGNAMENT");
  ASSERT(body->assignament.operator== FL_TK_EQUAL, "operator: FL_TK_EQUAL");

  ASSERT(body->assignament.left->type == FL_AST_LIT_IDENTIFIER,
         "left: FL_AST_LIT_IDENTIFIER");
  ASSERT(body->assignament.right->type == FL_AST_LIT_IDENTIFIER,
         "right: FL_AST_LIT_IDENTIFIER");

  fl_ast_delete(root);

  root = fl_parse_utf8("a =b; var i8 a; var i8 b;"); // ws test
  CHK_BODY(root, body);
  ASSERT(body->type == FL_AST_EXPR_ASSIGNAMENT, "FL_AST_EXPR_ASSIGNAMENT");
  fl_ast_delete(root);

  root = fl_parse_utf8("a = b; var i8 a; var i8 b;"); // ws test
  CHK_BODY(root, body);
  ASSERT(body->type == FL_AST_EXPR_ASSIGNAMENT, "FL_AST_EXPR_ASSIGNAMENT");
  fl_ast_delete(root);

  root = fl_parse_utf8("a= b; var i8 a; var i8 b;"); // ws test
  CHK_BODY(root, body);
  ASSERT(body->type == FL_AST_EXPR_ASSIGNAMENT, "FL_AST_EXPR_ASSIGNAMENT");
  fl_ast_delete(root);

  root = fl_parse_utf8("1 +2"); // ws test
  CHK_BODY(root, body);
  ASSERT(body->type == FL_AST_EXPR_BINOP, "FL_AST_EXPR_BINOP");
  fl_ast_delete(root);

  root = fl_parse_utf8("1 + 2"); // ws test
  CHK_BODY(root, body);
  ASSERT(body->type == FL_AST_EXPR_BINOP, "FL_AST_EXPR_BINOP");
  fl_ast_delete(root);

  // function call
  root = fl_parse_utf8("printf ( 'xxx' )"); // ws test
  CHK_BODY(root, body);
  ASSERT(body->type == FL_AST_EXPR_CALL, "FL_AST_EXPR_CALL");
  fl_ast_delete(root);

  // double assignament
  root = fl_parse_utf8("var x; var y; x = y = 1;"); // ws test
  CHK_BODY(root, body);
  fl_ast_delete(root);

  root = fl_parse_utf8("5 << 6"); // left shift
  CHK_BODY(root, body);
  ASSERT(body->type == FL_AST_EXPR_BINOP, "parsed");
  ASSERT(body->binop.left->type == FL_AST_LIT_NUMERIC, "left numeric");
  ASSERT(body->binop.right->type == FL_AST_LIT_NUMERIC, "right numeric");
  fl_ast_delete(root);

  // parethesis test!

  root = fl_parse_utf8("((5) << (6))"); // left shift
  CHK_BODY(root, body);

  ASSERT(body->type == FL_AST_EXPR_BINOP, "parsed");
  ASSERT(body->binop.left->type == FL_AST_LIT_NUMERIC, "left numeric");
  ASSERT(body->binop.right->type == FL_AST_LIT_NUMERIC, "right numeric");
  fl_ast_delete(root);

  root = fl_parse_utf8("1*(2+3)"); // priotity test
  CHK_BODY(root, body);
  ASSERT(body->type == FL_AST_EXPR_BINOP, "FL_AST_EXPR_BINOP");

  ASSERT(body->binop.left->type == FL_AST_LIT_NUMERIC, "FL_AST_LIT_NUMERIC");
  ASSERT(body->binop.operator== FL_TK_ASTERISK, "operator *");

  ASSERT(body->binop.right->binop.left->type == FL_AST_LIT_NUMERIC,
         "FL_AST_LIT_NUMERIC");
  ASSERT(body->binop.right->binop.operator== FL_TK_PLUS, "right operator +");
  ASSERT(body->binop.right->binop.right->type == FL_AST_LIT_NUMERIC,
         "FL_AST_LIT_NUMERIC");

  fl_ast_delete(root);

  root = fl_parse_utf8("1 | 2");
  CHK_BODY(root, body);
  ASSERT(body->type == FL_AST_EXPR_BINOP, "FL_AST_EXPR_BINOP");
  fl_ast_delete(root);

  root = fl_parse_utf8("5.0 == 5.0");
  CHK_BODY(root, body);
  ASSERT(body->type == FL_AST_EXPR_BINOP, "FL_AST_EXPR_BINOP");
  fl_ast_delete(root);

  root = fl_parse_utf8("var i32 i; i = 1; i == 5.0");
  CHK_BODY(root, body);
  fl_ast_delete(root);

  return 0;
}
