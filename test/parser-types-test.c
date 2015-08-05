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

void test_parser_type(fl_ast_t* body, size_t typeid) {
  fl_ast_t* var_type;

  ASSERT(body->type == FL_AST_DTOR_VAR, "first in body: FL_AST_DTOR_VAR");

  var_type = body->var.type;
  ASSERT(var_type != 0, "dtor has a type");
  ASSERTE(var_type->type, FL_AST_TYPE, "%d != %d", "dtor type is FL_AST_TYPE");
  ASSERTE(var_type->ty_id, typeid, "%zu != %zu", "typeid ?");
}

// TODO review if ";" is required
TASK_IMPL(parser_types) {
  log_debug_level = 0;

  TEST_PARSER_OK("bool type=2", "var bool hello;",
                 { test_parser_type(*body, 2); });

  TEST_PARSER_OK("i8 type=3", "var i8 hello;", { test_parser_type(*body, 3); });

  TEST_PARSER_OK("u8 type=4", "var u8 hello;", { test_parser_type(*body, 4); });

  TEST_PARSER_OK("i16 type=5", "var i16 hello;",
                 { test_parser_type(*body, 5); });

  TEST_PARSER_OK("i32 type=7", "var i32 hello;",
                 { test_parser_type(*body, 7); });

  TEST_PARSER_OK("i64 type=9", "var i64 hello;",
                 { test_parser_type(*body, 9); });

  TEST_PARSER_OK("f32 type=11", "var f32 hello;",
                 { test_parser_type(*body, 11); });

  TEST_PARSER_OK("f64 type=12", "var f64 hello;", {
    test_parser_type(*body, 12);

    // this must be tested inside, test macro clean types
    fl_type_t ty = fl_type_table[12];
    ASSERTE(ty.number.fp, true, "%d == %d", "type if fp");
    ASSERTE(ty.number.bits, 64, "%d == %d", "type is 64 bits");
  });

  TEST_PARSER_OK("custom type 01", "var ptr<f32> hello;",
                 { test_parser_type(*body, 13); });

  TEST_PARSER_OK("uniques 01", "var ptr<f32> a; var ptr<f32> b;", {
    test_parser_type(body[0], 13);
    test_parser_type(body[1], 13);
  });

  TEST_PARSER_OK("inference 01", "var x; x = 10;",
                 { test_parser_type(body[0], 9); });

  TEST_PARSER_OK("empty struct", "struct test {}",
                 { ASSERT(body[0]->ty_id == 13, "typeid 13"); });

  TEST_PARSER_OK("simple struct", "struct test {"
                                  "i8 t1"
                                  "}",
                 { ASSERT(body[0]->ty_id == 13, "typeid 13"); });

  TEST_PARSER_OK("complex struct", "struct test {"
                                   "i8 t1,"
                                   "i32 t2"
                                   "}",
                 { ASSERT(body[0]->ty_id == 13, "typeid 13"); });

  TEST_PARSER_OK("unique 02", "struct test {"
                              "i8 t1,"
                              "i32 t2"
                              "}"
                              "struct test2 {"
                              "i8 t1,"
                              "i32 t2"
                              "}",
                 {
                   ASSERT(body[0]->ty_id == 13, "typeid 13");
                   ASSERT(body[1]->ty_id == 13, "typeid 13");
                 });

  TEST_PARSER_OK("struct type usage", "struct test {"
                                      "i8 t1,"
                                      "i32 t2"
                                      "}"
                                      "struct test2 {"
                                      "test t"
                                      "}",
                 {
                   ASSERT(body[0]->ty_id == 13, "typeid 13");
                   ASSERT(body[1]->ty_id == 14, "typeid 14");
                 });

  return 0;
}
