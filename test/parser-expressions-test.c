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

  TEST_PARSER_OK("expressions 01", "1+2", {
    ASSERT(body[0]->type == FL_AST_EXPR_BINOP, "FL_AST_EXPR_BINOP");
    ASSERT(body[0]->binop.left->type == FL_AST_LIT_NUMERIC,
           "FL_AST_LIT_NUMERIC");
    ASSERT(body[0]->binop.right->type == FL_AST_LIT_NUMERIC,
           "FL_AST_LIT_NUMERIC");
  });
  TEST_PARSER_OK("expressions 02", "1+2+3", {
    ASSERT(body[0]->type == FL_AST_EXPR_BINOP, "FL_AST_EXPR_BINOP");
    ASSERT(body[0]->binop.left->type == FL_AST_LIT_NUMERIC,
           "FL_AST_LIT_NUMERIC");
    ASSERT(body[0]->binop.right->type == FL_AST_EXPR_BINOP,
           "FL_AST_EXPR_BINOP");
    ASSERT(body[0]->binop.right->binop.left->type == FL_AST_LIT_NUMERIC,
           "FL_AST_LIT_NUMERIC");
    ASSERT(body[0]->binop.right->binop.right->type == FL_AST_LIT_NUMERIC,
           "FL_AST_LIT_NUMERIC");
  });

  TEST_PARSER_OK("expressions 03", "1*2", {
    ASSERT(body[0]->type == FL_AST_EXPR_BINOP, "FL_AST_EXPR_BINOP");
    ASSERT(body[0]->binop.left->type == FL_AST_LIT_NUMERIC,
           "FL_AST_LIT_NUMERIC");
    ASSERT(body[0]->binop.operator== FL_TK_ASTERISK, "FL_TK_ASTERISK");
    ASSERT(body[0]->binop.right->type == FL_AST_LIT_NUMERIC,
           "FL_AST_LIT_NUMERIC");
  });

  TEST_PARSER_OK("expressions 04", "1*2+3", {
    ASSERT(body[0]->type == FL_AST_EXPR_BINOP, "FL_AST_EXPR_BINOP");

    ASSERT(body[0]->binop.left->binop.left->type == FL_AST_LIT_NUMERIC,
           "FL_AST_LIT_NUMERIC");
    ASSERT(body[0]->binop.left->binop.operator== FL_TK_ASTERISK,
           "left FL_TK_ASTERISK");
    ASSERT(body[0]->binop.left->binop.right->type == FL_AST_LIT_NUMERIC,
           "FL_AST_LIT_NUMERIC");

    ASSERT(body[0]->binop.operator== FL_TK_PLUS, "FL_TK_PLUS");
    ASSERT(body[0]->binop.right->type == FL_AST_LIT_NUMERIC,
           "FL_AST_LIT_NUMERIC");
  });

  TEST_PARSER_OK("unary 01", "-2", {
    ASSERT(body[0]->type == FL_AST_EXPR_LUNARY, "ast is FL_AST_EXPR_LUNARY");

    ASSERT(body[0]->lunary.element->type == FL_AST_LIT_NUMERIC,
           "FL_AST_LIT_NUMERIC");
    ASSERT(body[0]->lunary.operator== FL_TK_MINUS, "operator FL_TK_MINUS");
  });

  TEST_PARSER_OK("unary 02", "var i64 xxx; xxx++", {
    ASSERT(body[1]->type == FL_AST_EXPR_RUNARY, "ast is FL_AST_EXPR_RUNARY");

    ASSERT(body[1]->lunary.element->type == FL_AST_LIT_IDENTIFIER,
           "FL_AST_LIT_IDENTIFIER");
    ASSERT(body[1]->lunary.operator== FL_TK_PLUS2, "operator FL_TK_PLUS2");
  });

  // TODO nice hack for testing, but declaration first!
  TEST_PARSER_OK("assignament 01", "a=b; var i8 a; var i8 b;", {
    ASSERT(body[0]->type == FL_AST_EXPR_ASSIGNAMENT, "FL_AST_EXPR_ASSIGNAMENT");
    ASSERT(body[0]->assignament.operator== FL_TK_EQUAL,
           "operator: FL_TK_EQUAL");

    ASSERT(body[0]->assignament.left->type == FL_AST_LIT_IDENTIFIER,
           "left: FL_AST_LIT_IDENTIFIER");
    ASSERT(body[0]->assignament.right->type == FL_AST_LIT_IDENTIFIER,
           "right: FL_AST_LIT_IDENTIFIER");
  });

  TEST_PARSER_OK("assignament 01", "a =b; var i8 a; var i8 b;", {
    ASSERT(body[0]->type == FL_AST_EXPR_ASSIGNAMENT, "FL_AST_EXPR_ASSIGNAMENT");
  });

  TEST_PARSER_OK("assignament 02", "a = b; var i8 a; var i8 b;", {
    ASSERT(body[0]->type == FL_AST_EXPR_ASSIGNAMENT, "FL_AST_EXPR_ASSIGNAMENT");
  });

  TEST_PARSER_OK("assignament 03", "a= b; var i8 a; var i8 b;", {
    ASSERT(body[0]->type == FL_AST_EXPR_ASSIGNAMENT, "FL_AST_EXPR_ASSIGNAMENT");
  });

  TEST_PARSER_OK("binop 01", "1 +2", {
    ASSERT(body[0]->type == FL_AST_EXPR_BINOP, "FL_AST_EXPR_BINOP");
  });

  TEST_PARSER_OK("binop 01", "1 + 2", {
    ASSERT(body[0]->type == FL_AST_EXPR_BINOP, "FL_AST_EXPR_BINOP");
  });

  TEST_PARSER_OK("function call 01", "printf ( 'xxx' )", {
    ASSERT(body[0]->type == FL_AST_EXPR_CALL, "FL_AST_EXPR_CALL");
  });

  TEST_PARSER_OK("chained assignament", "var x; var y; x = y = 1;", {});

  TEST_PARSER_OK("left shift", "5 << 6", {
    ASSERT(body[0]->type == FL_AST_EXPR_BINOP, "parsed");
    ASSERT(body[0]->binop.left->type == FL_AST_LIT_NUMERIC, "left numeric");
    ASSERT(body[0]->binop.right->type == FL_AST_LIT_NUMERIC, "right numeric");
  });

  TEST_PARSER_OK("left shift (parethesis)", "((5) << (6))", {
    ASSERT(body[0]->type == FL_AST_EXPR_BINOP, "parsed");
    ASSERT(body[0]->binop.left->type == FL_AST_LIT_NUMERIC, "left numeric");
    ASSERT(body[0]->binop.right->type == FL_AST_LIT_NUMERIC, "right numeric");
  });

  TEST_PARSER_OK("expr priority (parethesis)", "1*(2+3)", {
    ASSERT(body[0]->type == FL_AST_EXPR_BINOP, "FL_AST_EXPR_BINOP");
    ASSERT(body[0]->binop.left->type == FL_AST_LIT_NUMERIC,
           "FL_AST_LIT_NUMERIC");
    ASSERT(body[0]->binop.operator== FL_TK_ASTERISK, "operator *");

    ASSERT(body[0]->binop.right->binop.left->type == FL_AST_LIT_NUMERIC,
           "FL_AST_LIT_NUMERIC");
    ASSERT(body[0]->binop.right->binop.operator== FL_TK_PLUS,
           "right operator +");
    ASSERT(body[0]->binop.right->binop.right->type == FL_AST_LIT_NUMERIC,
           "FL_AST_LIT_NUMERIC");
  });
  TEST_PARSER_OK("expr or", "1 | 2", {
    ASSERT(body[0]->type == FL_AST_EXPR_BINOP, "FL_AST_EXPR_BINOP");
  });

  TEST_PARSER_OK("equality", "5.0 == 5.0", {
    ASSERT(body[0]->type == FL_AST_EXPR_BINOP, "FL_AST_EXPR_BINOP");
  });

  TEST_PARSER_OK("equality 2", "var i32 i; i = 1; i == 5.0", {});

  // TODO illustrate typesystem vs inference war, fix it!
  TEST_PARSER_OK("member access", "struct xxx {i8 b};"
                                  "var xxx sin;"
                                  "var x;"
                                  "x = sin.b;",
                 {});
  TEST_PARSER_OK("member access",

                 "struct yyy {i64 hello};"
                 "var yyy s;"
                 "s.hello = 1;",
                 {
                   ASSERT(body[0]->ty_id == 13, "struct type 13");
                   ASSERT(body[1]->ty_id == 13, "variable type 13");
                   ASSERT(body[2]->ty_id == 9, "assignament type 9");
                 });
  /*
    TEST_PARSER_OK("member access", "var h; h = s.hello.world();", {});
    TEST_PARSER_OK("member access", "var h; h = s.hello.world(15, 20);", {});

    exit(1);
  */
  return 0;
}
