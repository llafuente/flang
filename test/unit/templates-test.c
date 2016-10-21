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

TASK_IMPL(parser_templates) {
  log_debug_level = 0;

  TEST_PARSER_ERROR("implement function, invalid arguments 001",
                    "ffi fn printf(ptr(i8) format, ...) : i32;\n"
                    "template $tpl;\n"
                    "fn tpl_fn($tpl a, $tpl b): $tpl {\n"
                    "  printf(\"sizeof %lu\n\", sizeof($tpl));\n"
                    "  return a + b;\n"
                    "}\n"
                    "implement tpl_fn(i64, i64) as tpl_fn_i64;\n"
                    "tpl_fn(1, 2);\n"
                    "tpl_fn_i64(1, 2);\n",
                    "type error, cannot find function or variable with given "
                    "name: 'tpl_fn_i64'",
                    {});

  TEST_PARSER_ERROR(
      "implement function, invalid arguments 002",
      "ffi fn printf(ptr(i8) format, ...) : i32;\n"
      "template $tpl;\n"
      "fn tpl_fn($tpl a, $tpl b): $tpl {\n"
      "  printf(\"sizeof %lu\n\", sizeof($tpl));\n"
      "  return a + b;\n"
      "}\n"
      "implement tpl_fn(i64) as tpl_fn_i64;\n",
      "syntax error, try to implement a function with 1 parameters, expected 2",
      {});

  TEST_PARSER_ERROR("implement function, invalid arguments 002",
                    "ffi fn printf(ptr(i8) format, ...) : i32;\n"
                    "template $tpl;\n"
                    "fn tpl_fn($tpl a, $tpl b): $tpl {\n"
                    "  printf(\"sizeof %lu\n\", sizeof($tpl));\n"
                    "  return a + b;\n"
                    "}\n"
                    "implement tpl_fn(i64, i32) as tpl_fn_i64;\n"
                    "tpl_fn(1, 2);\n",
                    "type error, try to implement '$tpl' with two different "
                    "types 'i32' and 'i64'",
                    {});

  // TEST_PARSER_OK(, {
  //});

  return 0;
}
