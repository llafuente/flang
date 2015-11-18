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
* THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS\" AND ANY EXPRESS OR
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

TASK_IMPL(codegen_functions) {
  log_debug_level = 0;

  TEST_CODEGEN_OK("cg function 01", "fn x(f64 arg1, f64 arg2) : f64 {"
                                    "  return arg1 + arg2;"
                                    "}",
                  {});

  TEST_CODEGEN_OK("cg function 02", "fn x(f64 arg1, f64 arg2) : f64 {"
                                    "  return arg1 + arg2;"
                                    "}"
                                    "var f64 sum; "
                                    "sum = x(1, 2);",
                  {});

  TEST_CODEGEN_OK("cg function 03", "printf(\"%s\\n\", \"hello\");",
                  { fl_to_bitcode(module, "hello-world.bc"); });

  // fl_interpreter(module);
  TEST_CODEGEN_OK("cg function 04",
                  "var ptr(i8) str; str = \"hello\"; printf(\"%s\\n\", str);",
                  { fl_to_bitcode(module, "hello-world.bc"); });

  TEST_CODEGEN_OK("function 05",
                  "ffi fn x( i8 arg1 , i8 arg2 ) : i8 ;"
                  "ffi fn printf2( ptr(i8) format, ... ) : void ;",
                  {});

  TEST_CODEGEN_OK("function 06", "fn x() : i8 {"
                                 "return 0; }",
                  {});

  TEST_CODEGEN_OK("function 06", "fn x() : i8 {"
                                 "return 0; } x();",
                  {});

  TEST_CODEGEN_OK("function 07", "fn ftype() : i8 {"
                                 "return 0;"
                                 "}"
                                 "var ftype y;"
                                 "y = &ftype;"
                                 "y();",
                  {});
  return 0;
}
