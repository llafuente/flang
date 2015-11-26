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
TASK_IMPL(codegen_expressions) {
  log_debug_level = 0;

  TEST_CODEGEN_OK("expr 01", "var i64 c; c = 1 + 2;", {});
  TEST_CODEGEN_OK("expr 02", "var string hello;", {});
  TEST_CODEGEN_OK("expr 03", "var i64 c; var i64 d; d=1+2; c = 1;", {});
  TEST_CODEGEN_OK("expr 04", "var i64 c; c = 1; c = 1;", {});
  TEST_CODEGEN_OK("expr 05", "var i64 a; var i64 b; a = b = 1;", {});
  TEST_CODEGEN_OK("expr 06", "var i64 a; var i64 b; a = 1; b = a + 1;", {});

  // TEST_CODEGEN_OK("expr 04", "printf(\"%f\", 1 + 2);", {});

  TEST_CODEGEN_OK("type infer 01", "var x; var y; x = y = 1;", {});
  TEST_CODEGEN_OK("type infer 02", "var x; var y; x = 1; y = x + 1;", {});
  TEST_CODEGEN_OK("type infer 03", "fn a(i8 b) { return 0; } var x; a(x);", {});
  TEST_CODEGEN_OK("type infer 04",
                  "fn a(i8 b) : i8 { var x = b; return x; } var x; a(x);", {});

  TEST_CODEGEN_OK("type cg", "struct stt {i8 b,};"
                             "fn func(i8 b) : i8 { return b; }"
                             "var stt ins;"
                             "ins.b = 1;"
                             "func(ins.b);",
                  {});

  TEST_CODEGEN_OK("type cg2", "struct stt {i8 b,};"
                              "fn func(i8 b) : i8 { return b; }"
                              "var ptr(stt) ins;"
                              "ins = malloc(8);"
                              "ins[0].b = 1;"
                              "func(ins[0].b);",
                  {});

  log_debug_level = 10;
  TEST_CODEGEN_OK("unary 01", "var i64 c = 0; c++;", {});
  TEST_CODEGEN_OK("unary 02", "var i64 c = 0; ++c;", {});
  TEST_CODEGEN_OK("unary 03", "var i64 c = 0; c--;", {});
  TEST_CODEGEN_OK("unary 04", "var i64 c = 0; --c;", {});

  return 0;
}
