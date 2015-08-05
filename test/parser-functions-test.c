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
    ASSERT(body[0]->func.nparams == 0, "no args");
  });

  TEST_PARSER_OK("function 02", "fn x(yy, zz , mm ,xx) {}", {
    ASSERT(body[0]->type == FL_AST_DECL_FUNCTION, "FL_AST_DECL_FUNCTION");
    ASSERT(body[0]->func.id->type == FL_AST_LIT_IDENTIFIER,
           "FL_AST_LIT_IDENTIFIER");
    ASSERT(body[0]->func.params != 0, "no args");
  });

  TEST_PARSER_ERROR("function err 01", "fn {}",
                    "cannot parse function identifier",
                    { CHK_ERROR_RANGE(err, 4, 1, 5, 1); });

  TEST_PARSER_ERROR("function err 01", "fn hell ({}", "expected identifier",
                    {});

  TEST_PARSER_ERROR("function err 01", "fn x a", "expected '('",
                    { CHK_ERROR_RANGE(err, 6, 1, 7, 1); });

  TEST_PARSER_ERROR("function err 01", "fn (){};",
                    "cannot parse function identifier",
                    { CHK_ERROR_RANGE(err, 4, 1, 5, 1); });

  TEST_PARSER_ERROR("function err 01", "fn x () { fn (){}; }",
                    "cannot parse function identifier",
                    { CHK_ERROR_RANGE(err, 14, 1, 15, 1); });

  // TODO 'template'
  TEST_PARSER_OK("function 03", "fn x(i8 arg1, i8 arg2) {"
                                "return arg1 + arg2;"
                                "}",
                 {});

  // declaration only
  TEST_CODEGEN_OK("function 04", "fn x( i8 arg1 , i8 arg2 ) : i8 ;"
                                 "fn printf2( ptr<i8> format, ... ) ;",
                  {});

  return 0;
}
