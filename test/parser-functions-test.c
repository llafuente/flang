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
  log_debug_level = 0;

  TEST_PARSER_OK("function 01", "fn x() {}", {
    ASSERT(body[0]->type == FL_AST_DECL_FUNCTION, "FL_AST_DECL_FUNCTION");
    ASSERT(body[0]->func.id->type == FL_AST_LIT_IDENTIFIER,
           "FL_AST_LIT_IDENTIFIER");
    ASSERT(body[0]->func.params->list.count == 0, "no args");
  });

  TEST_PARSER_OK("function 02", "fn x(yy, zz , mm ,xx) {}", {
    ASSERT(body[0]->type == FL_AST_DECL_FUNCTION, "FL_AST_DECL_FUNCTION");
    ASSERT(body[0]->func.id->type == FL_AST_LIT_IDENTIFIER,
           "FL_AST_LIT_IDENTIFIER");
    ASSERT(body[0]->func.params != 0, "no args");
  });
  TEST_PARSER_ERROR("function err 01", "fn {} var x;",
                    "syntax error, unexpected '{', expecting AST_IDENT",
                    {/*CHK_ERROR_RANGE(err, 4, 1, 5, 1);*/});

  TEST_PARSER_ERROR(
      "function err 02", "fn hell ({}",
      "syntax error, unexpected '{', expecting TK_ANY or AST_IDENT or ')'", {});

  TEST_PARSER_ERROR("function err 03", "fn x a",
                    "syntax error, unexpected AST_IDENT, expecting '{' or ':'",
                    {/*CHK_ERROR_RANGE(err, 6, 1, 9, 1);*/});

  TEST_PARSER_ERROR("function err 04", "fn (){};",
                    "syntax error, unexpected '(', expecting AST_IDENT",
                    {/*CHK_ERROR_RANGE(err, 4, 1, 5, 1);*/});

  TEST_PARSER_ERROR("function err 05", "fn x () { fn (){}; }",
                    "syntax error, unexpected '(', expecting AST_IDENT",
                    {/*CHK_ERROR_RANGE(err, 14, 1, 15, 1);*/});

  TEST_PARSER_ERROR(
      "function err 05", "ffi fn x () { var x; }",
      "syntax error, ffi cannot have a body and must have declared return type",
      {});

  TEST_PARSER_ERROR("function err 05", "ffi fn x () : i32 { var x; }",
                    "syntax error, ffi cannot have a body", {});
  // TODO 'template'
  TEST_PARSER_OK("function 03", "var i8 zz; fn x(i8 arg1, i8 arg2) : i8 {"
                                "return arg1 + arg2;"
                                "}",
                 {});

  /* declaration only is for ffi
  TEST_PARSER_OK("function 04", "fn x( i8 arg1 , i8 arg2 ) : i8 ;"
                                "fn printf2( ptr(i8) format, ... ) ;",
                 {});
  */

  TEST_PARSER_OK("function 05", "function test_i32() {}", {});

  TEST_PARSER_OK("function 06", "function t(i8 i) {} var i32 i; i = 0;", {
    ASSERT(body[2]->assignament.left->ty_id == TS_I32, "i is i32");
  });

  TEST_PARSER_OK("function 07", "ffi fn printf(ptr(i8) format) : i32;",
                 { ASSERT(body[0]->func.ffi == true, "function is ffi"); });

  TEST_PARSER_OK("function 07", "ffi fn printf(ptr(i8) format, ...) : i32;", {
    ASSERT(body[0]->func.ffi == true, "function is ffi");
    ASSERT(body[0]->func.varargs == true, "function is varargs");
  });

  return 0;
}
