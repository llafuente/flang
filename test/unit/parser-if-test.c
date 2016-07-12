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

TASK_IMPL(parser_if) {
  log_debug_level = 0;

  TEST_PARSER_OK("function 01", "var bool b; b = 1;"
                                "if b == true { \"ok\"; }",
                 {});

  TEST_PARSER_OK("function 01", "if(true) {}", {});

  TEST_PARSER_ERROR("function 01", "if(true)",
                    "syntax error, unexpected $end, expecting '{'", {});

  TEST_PARSER_ERROR("function 01", "if(true) {",
                    "syntax error, unexpected $end, expecting '}'", {});

  TEST_PARSER_ERROR("function 01", "if(true) {/}",
                    "syntax error, unexpected '/', expecting '}'", {

                                                                   });

  TEST_PARSER_OK("function 01", "var bool b; b = 1;\n"
                                "if (b == true) { \"ok\"; }\n"
                                "else { \"ko\"; }",
                 {});

  TEST_PARSER_OK("function 01", "var i32 b; b = 1;\n"
                                "if (b == 2) { \"2\"; }\n"
                                "else if (b == 1) { \"1\"; }",
                 {});

  return 0;
}
