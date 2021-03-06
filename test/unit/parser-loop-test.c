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

TASK_IMPL(parser_loops) {
  log_debug_level = 0;

  TEST_PARSER_OK("loop 01", "var i32 x; for x = 1; x < 10; ++x {"
                            "}",
                 {});

  TEST_PARSER_ERROR("loop 01", "var i32 x; for x = 1; x < 10; ++x ",
                    "syntax error, unexpected end of file, expecting '{'", {});

  TEST_PARSER_ERROR("loop 01", "var i32 x; for x = 1; x < 10;",
                    //"expected update expression", {});
                    "syntax error, unexpected end of file", {});

  TEST_PARSER_ERROR("loop 01", "var i32 x; for x = 1; x < 10",
                    //"expected semicolon", {});
                    "syntax error, unexpected end of file, expecting ';'", {});

  TEST_PARSER_ERROR("loop 01", "var i32 x; for x = 1;",
                    //"expected condition expression", {});
                    "syntax error, unexpected identifier, expecting ';'", {});

  TEST_PARSER_ERROR("loop 01", "var i32 x; for x = 1",
                    "syntax error, unexpected end of file, expecting ';'", {});

  TEST_PARSER_ERROR("loop 01", "var i32 x; for ",
                    //"expected initialization expression", {});
                    "syntax error, unexpected identifier, expecting ';'", {});

  TEST_PARSER_ERROR(
      "loop 01", "var i32 x; var z; for x = 1; x < (z = 10); ++x {"
                 "}",
      "syntax error, assignament expression is fobidden here.", {});

  // TODO this should be valid!?
  TEST_PARSER_ERROR("loop 01", "var i32 x = 1; for ; x < 10; ++x {"
                               "}",
                    "syntax error, unexpected ';'", {});

  // TODO this should be valid!?
  TEST_PARSER_ERROR("loop 01", "for var i32 x = 1; x < 10; ++x {"
                               "}",
                    "syntax error, unexpected var", {});

  TEST_PARSER_OK("loop 01", "var i32 x; x = 1; while x < 10 {"
                            "++x;"
                            "}",
                 {});

  TEST_PARSER_ERROR("loop 01", "var i32 x; x = 1; while x < 10 {"
                               "++x;",
                    "syntax error, unexpected end of file, expecting '}'", {});

  TEST_PARSER_ERROR("loop 01", "var i32 x; x = 1; while x < 10",
                    "syntax error, unexpected end of file, expecting '{'", {});

  TEST_PARSER_ERROR("loop 01", "var i32 x; x = 1; while",
                    "syntax error, unexpected identifier, expecting ';'", {});

  TEST_PARSER_OK("loop 01", "var i32 x; x = 1; do {"
                            "++x;"
                            "} while x < 10;",
                 {});

  TEST_PARSER_ERROR("loop 01", "var i32 x; var z; x = 1; do {"
                               "++x;"
                               "} while x < (z = 10);",
                    "syntax error, assignament expression is fobidden here.",
                    {});

  TEST_PARSER_ERROR("loop 01", "var i32 x; x = 1; do {"
                               "++x;"
                               "} while ",
                    "syntax error, unexpected end of file", {});

  TEST_PARSER_ERROR("loop 01", "var i32 x; x = 1; do {"
                               "++x;"
                               "}",
                    "syntax error, unexpected end of file, expecting while",
                    {});

  TEST_PARSER_ERROR("loop 01", "var i32 x; x = 1; do {"
                               "++x;",
                    "syntax error, unexpected end of file, expecting '}'", {});

  TEST_PARSER_ERROR("loop 01", "var i32 x; x = 1; do ",
                    "syntax error, unexpected end of file, expecting '{'", {});

  return 0;
}
