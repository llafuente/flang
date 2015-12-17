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
  log_debug_level = 0;

  /*
  TEST_PARSER_OK("sizeof parse",
      "var i32 a = 27; var i32 b = 3; var i32 c = 3;"
            "a / b / c;", {
    ast_dump(root);
  });
  */

  TEST_PARSER_OK("sizeof parse", "sizeof(i8);", {
    ASSERT(body[0]->type == FL_AST_EXPR_SIZEOF, "is sizeof");
    ASSERT(body[0]->ty_id == TS_I64, "type is i64");
    ASSERT(body[0]->sof.type->ty_id == TS_I8, "type is i8");
  });

  TEST_PARSER_OK("parent till root works", "1 + 2;", {
    ASSERT(body[0]->parent != 0, "has parent");
    ASSERT(body[0]->parent == root->program.body->block.body, "is parent");
    ASSERT(body[0]->parent->parent != 0, "has parent");
    ASSERT(body[0]->parent->parent == root->program.body, "is parent");
    ASSERT(body[0]->parent->parent->parent != 0, "has parent");
    ASSERT(body[0]->parent->parent->parent == root, "is parent");
  });

  TEST_PARSER_OK("fix missing operator ", "1 <= 2;", {
    ASSERT(body[0]->type == FL_AST_EXPR_BINOP, "FL_AST_EXPR_BINOP");
    ASSERT(body[0]->binop.operator== TK_LE, "operator TK_LE");
  });

  TEST_PARSER_OK("fix left recursion", "1 + 2 + 3 + 4;", {
    ASSERT(body[0]->type == FL_AST_EXPR_BINOP, "FL_AST_EXPR_BINOP");
    ASSERT(body[0]->binop.left->type == FL_AST_EXPR_BINOP,
           "left is FL_AST_EXPR_BINOP");
  });

  TEST_PARSER_OK("expressions 01", "1+2;", {
    ASSERT(body[0]->type == FL_AST_EXPR_BINOP, "FL_AST_EXPR_BINOP");
    ASSERT(body[0]->binop.left->type == FL_AST_LIT_INTEGER,
           "FL_AST_LIT_INTEGER");
    ASSERT(body[0]->binop.right->type == FL_AST_LIT_INTEGER,
           "FL_AST_LIT_INTEGER");
  });
  TEST_PARSER_OK("expressions 02", "1+2+3;", {
    ASSERT(body[0]->type == FL_AST_EXPR_BINOP, "FL_AST_EXPR_BINOP");
    ASSERT(body[0]->binop.right->type == FL_AST_LIT_INTEGER,
           "FL_AST_LIT_INTEGER");
    ASSERT(body[0]->binop.left->type == FL_AST_EXPR_BINOP, "FL_AST_EXPR_BINOP");
    ASSERT(body[0]->binop.left->binop.left->type == FL_AST_LIT_INTEGER,
           "FL_AST_LIT_INTEGER");
    ASSERT(body[0]->binop.left->binop.right->type == FL_AST_LIT_INTEGER,
           "FL_AST_LIT_INTEGER");
  });
  TEST_PARSER_OK("expressions 03", "1*2;", {
    ASSERT(body[0]->type == FL_AST_EXPR_BINOP, "FL_AST_EXPR_BINOP");
    ASSERT(body[0]->binop.left->type == FL_AST_LIT_INTEGER,
           "FL_AST_LIT_INTEGER");
    ASSERT(body[0]->binop.operator== '*', "'*'");
    ASSERT(body[0]->binop.right->type == FL_AST_LIT_INTEGER,
           "FL_AST_LIT_INTEGER");
  });

  TEST_PARSER_OK("expressions 04", "1*2+3;", {
    ASSERT(body[0]->type == FL_AST_EXPR_BINOP, "FL_AST_EXPR_BINOP");

    ASSERT(body[0]->binop.left->binop.left->type == FL_AST_LIT_INTEGER,
           "FL_AST_LIT_INTEGER");
    ASSERT(body[0]->binop.left->binop.operator== '*', "left '*'");
    ASSERT(body[0]->binop.left->binop.right->type == FL_AST_LIT_INTEGER,
           "FL_AST_LIT_INTEGER");

    ASSERT(body[0]->binop.operator== '+', "'+'");
    ASSERT(body[0]->binop.right->type == FL_AST_LIT_INTEGER,
           "FL_AST_LIT_INTEGER");
  });

  TEST_PARSER_OK("unary 01", "-2;", {
    ASSERT(body[0]->type == FL_AST_EXPR_LUNARY, "ast is FL_AST_EXPR_LUNARY");

    ASSERT(body[0]->lunary.element->type == FL_AST_LIT_INTEGER,
           "FL_AST_LIT_INTEGER");
    ASSERT(body[0]->lunary.operator== '-', "operator '-'");
  });

  TEST_PARSER_OK("unary 02", "var i64 xxx; xxx++;", {
    ASSERT(body[1]->type == FL_AST_EXPR_RUNARY, "ast is FL_AST_EXPR_RUNARY");

    ASSERT(body[1]->lunary.element->type == FL_AST_LIT_IDENTIFIER,
           "FL_AST_LIT_IDENTIFIER");
    ASSERT(body[1]->lunary.operator== TK_PLUSPLUS, "operator TK_PLUSPLUS");
  });

  TEST_PARSER_OK("assignament 01", "var i8 a; var i8 b; a=b;", {
    ASSERT(body[2]->type == FL_AST_EXPR_ASSIGNAMENT, "FL_AST_EXPR_ASSIGNAMENT");
    ASSERT(body[2]->assignament.operator== '=', "operator: '='");

    ASSERT(strcmp(body[0]->var.id->identifier.string->value, "a") == 0,
           "check id 0");
    ASSERT(strcmp(body[1]->var.id->identifier.string->value, "b") == 0,
           "check id 1");
    ASSERT(body[2]->assignament.left->type == FL_AST_LIT_IDENTIFIER,
           "left: FL_AST_LIT_IDENTIFIER");
    ASSERT(body[2]->assignament.right->type == FL_AST_LIT_IDENTIFIER,
           "right: FL_AST_LIT_IDENTIFIER");
  });

  TEST_PARSER_OK("assignament 01", "global i8 a = 1; fn abc() { return a; }",
                 {});

  TEST_PARSER_OK("assignament 01", "var i8 a; var i8 b; a =b;", {
    ASSERT(body[2]->type == FL_AST_EXPR_ASSIGNAMENT, "FL_AST_EXPR_ASSIGNAMENT");
  });

  TEST_PARSER_OK("assignament 02", "var i8 a; var i8 b; a = b;", {
    ASSERT(body[2]->type == FL_AST_EXPR_ASSIGNAMENT, "FL_AST_EXPR_ASSIGNAMENT");
  });

  TEST_PARSER_OK("assignament 03", "var i8 a; var i8 b; a= b;", {
    ASSERT(body[2]->type == FL_AST_EXPR_ASSIGNAMENT, "FL_AST_EXPR_ASSIGNAMENT");
  });

  TEST_PARSER_OK("binop 01", "1 +2;", {
    ASSERT(body[0]->type == FL_AST_EXPR_BINOP, "FL_AST_EXPR_BINOP");
  });

  TEST_PARSER_OK("binop 01", "1 + 2;", {
    ASSERT(body[0]->type == FL_AST_EXPR_BINOP, "FL_AST_EXPR_BINOP");
  });

  // TODO fix, should be string type
  TEST_PARSER_OK(
      "function call 01", "fn printf(ptr(i8) str) {}"
                          "printf ( \"xxx\" );",
      { ASSERT(body[1]->type == FL_AST_EXPR_CALL, "FL_AST_EXPR_CALL"); });

  TEST_PARSER_OK("chained assignament", "var x; var y; x = y = 1;", {});

  TEST_PARSER_OK("left shift", "5 << 6;", {
    ASSERT(body[0]->type == FL_AST_EXPR_BINOP, "parsed");
    ASSERT(ast_is_static(body[0]->binop.left) == true, "left is static");
    ASSERT(ast_is_static(body[0]->binop.right) == true, "right is static");
    ASSERT(body[0]->binop.left->type == FL_AST_LIT_INTEGER, "left numeric");
    ASSERT(body[0]->binop.right->type == FL_AST_LIT_INTEGER, "right numeric");
  });

  TEST_PARSER_OK("left shift (parethesis)", "((5) << (6));", {
    ASSERT(body[0]->type == FL_AST_EXPR_BINOP, "parsed");
    ASSERT(body[0]->binop.left->type == FL_AST_LIT_INTEGER, "left numeric");
    ASSERT(body[0]->binop.right->type == FL_AST_LIT_INTEGER, "right numeric");
  });

  TEST_PARSER_OK("expr priority (parethesis)", "1*(2+3);", {
    ASSERT(body[0]->type == FL_AST_EXPR_BINOP, "FL_AST_EXPR_BINOP");
    ASSERT(body[0]->binop.left->type == FL_AST_LIT_INTEGER,
           "FL_AST_LIT_INTEGER");
    ASSERT(body[0]->binop.operator== '*', "operator *");

    ASSERT(body[0]->binop.right->binop.left->type == FL_AST_LIT_INTEGER,
           "FL_AST_LIT_INTEGER");
    ASSERT(body[0]->binop.right->binop.operator== '+', "right operator +");
    ASSERT(body[0]->binop.right->binop.right->type == FL_AST_LIT_INTEGER,
           "FL_AST_LIT_INTEGER");
  });
  TEST_PARSER_OK("expr or", "1 | 2;", {
    ASSERT(body[0]->type == FL_AST_EXPR_BINOP, "FL_AST_EXPR_BINOP");
  });

  TEST_PARSER_OK("equality", "5.0 == 5.0;", {
    ASSERT(body[0]->type == FL_AST_EXPR_BINOP, "FL_AST_EXPR_BINOP");
  });

  // TODO test this err
  // TEST_PARSER_OK("equality 2", "var i32 i;\ni = 1;\ni == 5.0;", {});

  TEST_PARSER_OK("equality 3", "var i32 i;\ni = 1;\ni == 5;", {});

  // TODO illustrate typesystem vs inference war, fix it!
  TEST_PARSER_OK("member access", "struct xxx {i8 b};"
                                  "var xxx sin;"
                                  "var x;"
                                  "x = sin.b;",
                 {});
  TEST_PARSER_OK("member access", "struct yyy {i64 hello};"
                                  "var yyy s;"
                                  "s.hello = 1;",
                 {
                   fprintf(stderr, "# type: %zu\n", body[1]->ty_id);
                   /* TODO
                   ASSERT(body[1]->ty_id == TEST_TYPEID, "variable type");
                   ASSERT(body[2]->ty_id == TS_I64, "assignament type TS_I64");
                   */
                 });
  /*
    // TODO fix expr call with member / not identifier
    TEST_PARSER_OK("member access", "var h; h = s.hello.world();", {});
    TEST_PARSER_OK("member access", "var h; h = s.hello.world(15, 20);", {});

    exit(1);
  */

  TEST_PARSER_OK("array access", "var ptr(i64) x;"
                                 "x[0];"
                                 "&x;",
                 {
                  /** TODO
                  ASSERT(body[0]->ty_id == TEST_TYPEID, "struct type");
                  ASSERT(body[1]->ty_id == TS_I64, "dereference type TS_I64");
                  */
                 });

  TEST_PARSER_OK("fix unnecesary castings", "var i32 x;"
                                            "x = x + 1;",
                 {
                   // TODO ASSERT(body[1]->ty_id == TS_I32, "assignament type
                   // TS_I32");
                   ASSERT(body[1]->type == FL_AST_EXPR_ASSIGNAMENT,
                          "1st FL_AST_EXPR_ASSIGNAMENT");
                   ASSERT(body[1]->assignament.left->type ==
                              FL_AST_LIT_IDENTIFIER,
                          "left FL_AST_LIT_IDENTIFIER");
                   ASSERT(body[1]->assignament.right->type == FL_AST_EXPR_BINOP,
                          "right FL_AST_EXPR_BINOP");
                   ASSERT(body[1]->assignament.right->binop.left->type ==
                              FL_AST_LIT_IDENTIFIER,
                          "right.left FL_AST_LIT_IDENTIFIER");
                   ASSERT(body[1]->assignament.right->binop.right->type ==
                              FL_AST_LIT_INTEGER,
                          "right.left FL_AST_LIT_INTEGER");
                 });

  TEST_PARSER_OK("expressions 04", "1+2+3;", {
    ast_t* expr = body[0];

    ASSERT(expr->type == FL_AST_EXPR_BINOP, "root is binop");

    ast_t* expr_l = body[0]->binop.left;
    ast_t* expr_r = body[0]->binop.right;

    ASSERT(expr_l->type == FL_AST_EXPR_BINOP, "left is binop");
    ASSERT(expr_r->type == FL_AST_LIT_INTEGER, "left id literal");
  });

  TEST_PARSER_ERROR("type demotion", "var i32 a = 1;\n"
                                     "var i64 b = 1;\n"
                                     "a = b;",
                    "manual casting is required from i64 to i32", {});

  return 0;
}
