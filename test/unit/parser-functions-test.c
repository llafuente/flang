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

#include "../tasks.h"
#include "../test.h"

TASK_IMPL(parser_functions) {
  log_debug_level = 0;

  TEST_PARSER_OK("function 01", "fn x() {}", {
    ASSERT(body[0]->type == AST_DECL_FUNCTION, "AST_DECL_FUNCTION");
    ASSERT(body[0]->func.id->type == AST_LIT_IDENTIFIER, "AST_LIT_IDENTIFIER");
    ASSERT(body[0]->func.params->list.length == 0, "no args");
  });
  /* TODO inference should work here!
    TEST_PARSER_OK("function 02", "fn x(yy, zz , mm ,xx) {\n"
      "yy = 1.0;\n"
      "zz = \"string\";\n"
      "mm = 5;\n"
      "xx = 0;\n"
      "}", {
      ASSERT(body[0]->type == AST_DECL_FUNCTION, "AST_DECL_FUNCTION");
      ASSERT(body[0]->func.id->type == AST_LIT_IDENTIFIER,
    "AST_LIT_IDENTIFIER");
      ASSERT(body[0]->func.params != 0, "no args");
    });
  */
  TEST_PARSER_ERROR(
      "function err 01", "fn {} var x;",
      "syntax error, unexpected '{', expecting TK_OPERATOR or IDENTIFIER",
      {/*CHK_ERROR_RANGE(err, 4, 1, 5, 1);*/
      });

  TEST_PARSER_ERROR("function err 02", "fn hell ({}",
                    "syntax error, unexpected '{', expecting TK_ANY or TK_REF "
                    "or IDENTIFIER or ')'",
                    {});

  TEST_PARSER_ERROR("function err 03", "fn x a",
                    "syntax error, unexpected IDENTIFIER, expecting '{' or ':'",
                    {/*CHK_ERROR_RANGE(err, 6, 1, 9, 1);*/
                    });

  TEST_PARSER_ERROR(
      "function err 04", "fn (){};",
      "syntax error, unexpected '(', expecting TK_OPERATOR or IDENTIFIER",
      {/*CHK_ERROR_RANGE(err, 4, 1, 5, 1);*/
      });

  TEST_PARSER_ERROR(
      "function err 05", "fn x () { fn (){}; }",
      "syntax error, unexpected '(', expecting TK_OPERATOR or IDENTIFIER",
      {/*CHK_ERROR_RANGE(err, 14, 1, 15, 1);*/
      });

  TEST_PARSER_ERROR("function err 05-a", "ffi fn x () { var x; }",
                    "syntax error, ffi function require a return type", {});

  TEST_PARSER_ERROR(
      "function err 05-b", "ffi fn x () : i8 { var x; }",
      "syntax error, ffi cannot have a body and must have declared return type",
      {});

  // TODO maybe we can be more specific
  // TODO why this not fail? "fn x( i8 arg1 , i8 arg2 ) : i8 ;",
  // declaration only is for ffi
  TEST_PARSER_ERROR("function err 06", "fn printf2( ptr(i8) format, ... ) ;",
                    "syntax error, unexpected ';', expecting '{' or ':'", {});

  // TODO 'template'
  TEST_PARSER_OK("function 03", "var i8 zz; fn x(i8 arg1, i8 arg2) : i8 {"
                                "return arg1 + arg2;"
                                "}",
                 {});

  TEST_PARSER_OK("function 05", "function test_i32() {}", {});

  TEST_PARSER_OK("function 06", "function t(i8 i) {} var i32 i; i = 0;", {
    ASSERT(body[2]->assignament.left->ty_id == TS_I32, "i is i32");
  });

  TEST_PARSER_OK("function 07", "ffi fn printf(ptr(i8) format) : i32;",
                 { ASSERT(body[0]->func.ffi == true, "function is ffi"); });

  TEST_PARSER_OK("function 08", "ffi fn printf(ptr(i8) format, ...) : i32;", {
    ASSERT(body[0]->func.ffi == true, "function is ffi");
    ASSERT(body[0]->func.varargs == true, "function is varargs");
  });
  /*
  TEST_PARSER_OK("poly 01", "#id=sum_i32\n"
                            "fn sum(i32 a, i32 b) : i32 {"
                            "  return a + b;"
                            "}"
                            "#id=sum_i8\n"
                            "fn sum(i8 a, i8 b) : i8 {"
                            "  return a + b;"
                            "}",
                 {
    ASSERT(body[0]->func.ffi == false, "function is not ffi");
    ASSERT(body[0]->func.varargs == false, "function is not varargs");
    ASSERT(strcmp(body[0]->func.uid->value, "sum_i32") == 0,
           "function uid is what we manually set");

    ASSERT(body[1]->func.ffi == false, "function is not ffi");
    ASSERT(body[1]->func.varargs == false, "function is not varargs");
    ASSERT(strcmp(body[1]->func.uid->value, "sum_i8") == 0,
           "function uid is what we manually set");
  });
  */

  TEST_PARSER_ERROR("function err 05", "fn x (i8 a) : i32 { var x; }",
                    "Variable name 'x' in use by a type, previously defined at "
                    "memory:string:1:1",
                    {});

  TEST_PARSER_ERROR("function err 05", "fn x (i8 x) : i32 { var z; }",
                    "Parameter name 'x' in use by a type, previously defined "
                    "at memory:string:1:1",
                    {});

  TEST_PARSER_ERROR("function err 05", "fn x (i8 a, i8 a) : i32 { var z; }",
                    "Parameter name 'a' in use by a variable, previously "
                    "defined at memory:string:1:7",
                    {});

  TEST_PARSER_OK("template 01", "template $t;\n"
                                //"#id=sum_i32\n"
                                "fn sum($t a, i8 b) : i8 {"
                                "  return a + b;"
                                "}"
                                "var $t tpl;",
                 {
    ASSERT(body[0]->tpl.id->ty_id ==
               body[1]->func.params->list.values[0]->ty_id,
           "tpl type param");

    ASSERT(body[0]->tpl.id->ty_id == body[2]->ty_id, "tpl type var");
  });

  TEST_PARSER_ERROR("type demotion", "template $t;\n"
                                     "fn sum ($t __a, $t __b) : $t {\n"
                                     "  return __a + __b;\n"
                                     "}\n"
                                     "implement sum(i8, i8) as sum_i8;\n"
                                     "fn sum_i8 () {}\n",
                    "Function #id collision found for 'sum_i8', previously "
                    "used at memory:string:6:1",
                    {});

  TEST_PARSER_ERROR("type demotion", "template $t;\n"
                                     "fn sum ($t __a, $t __b) : $t {\n"
                                     "  return __a + __b;\n"
                                     "}\n"
                                     "implement sum(i8, i8) as sum_i8;\n"
                                     "implement sum(i16, i16) as sum_i8;\n",
                    "Function #id collision found for 'sum_i8', previously "
                    "used at memory:string:2:1",
                    {});

  return 0;
}
