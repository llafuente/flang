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

TASK_IMPL(parser_scopes) {
  log_debug_level = 0;

  TEST_PARSER_OK("scope 01 - test", "var a; var b; var c;"
                                    "struct x { i8 b };",
                 {
    ASSERT(hash_has(mainblock->block.variables, "a"),
           "a exists in global scope");
    ASSERT(hash_has(mainblock->block.variables, "b"),
           "b exists in global scope");
    ASSERT(hash_has(mainblock->block.variables, "c"),
           "c exists in global scope");

    ASSERT(hash_has(mainblock->block.types, "x"),
           "x type exists in global scope");
  });

  TEST_PARSER_OK("scope 02 - != scopes", "{ var a = 1; } {var a = 2;}", {
    ASSERT(hash_has(body[0]->block.variables, "a"), "a exists in first block");
    ASSERT(hash_has(body[1]->block.variables, "a"), "a exists in second block");
  });

  TEST_PARSER_OK("scope 03 - global", "{ global i32 a = 1; }", {
    ASSERT(hash_has(mainblock->block.variables, "a"),
           "a exists in global scope");
  });

  TEST_PARSER_OK("scope 04 - function", "fn x (i8 a, i8 b) {"
                                        "var i8 c;"
                                        "}",
                 {
    ASSERT(hash_has(body[0]->func.body->block.variables, "c"),
           "c exists in fn block");
    ASSERT(hash_has(body[0]->func.body->block.variables, "a"),
           "a exists in fn block");
    ASSERT(hash_has(body[0]->func.body->block.variables, "b"),
           "b exists in fn block");
  });

  TEST_PARSER_OK("scope 04 - function", "{"
                                        "struct x { i8 b };"
                                        "var x c;"
                                        "}{"
                                        "struct x { i8 b };"
                                        "var x c;"
                                        "}",
                 {
    ASSERT(hash_has(body[0]->block.variables, "c"),
           "c var exists in 1st block");
    ASSERT(hash_has(body[0]->block.types, "x"), "x type exists in 1st block");
    ASSERT(hash_has(body[1]->block.variables, "c"),
           "c var exists in 2nd block");
    ASSERT(hash_has(body[1]->block.types, "x"), "x type exists in 2nd block");

    ast_t* t1 = (ast_t*)hash_get(body[0]->block.types, "x");
    ast_t* t2 = (ast_t*)hash_get(body[1]->block.types, "x");
    ASSERT(body[0]->block.body->list.elements[1]->ty_id == t1->ty_id,
           "1st block c type ok");
    ASSERT(body[1]->block.body->list.elements[1]->ty_id == t2->ty_id,
           "2nd block c type ok");
  });

  TEST_PARSER_OK("scope 05 - polymorh", "fn x (i8 a, i8 b) {}"
                                        "fn x (i32 a, i32 b) {}",
                 {
    ASSERT(hash_has(mainblock->block.functions, "x"), "x exists in fn block");
    ASSERT(hash_has(mainblock->block.types, "x"), "x exists in fn block");
  });

  TEST_PARSER_OK("scope 06 - two decls", "function test_i32() {}\n"
                                         "function test_i64() {}\n"
                                         "test_i32();\n"
                                         "test_i64();\n",
                 {
    ASSERT(hash_has(mainblock->block.functions, "test_i32"),
           "test_i32 exists in fn block");
    ASSERT(hash_has(mainblock->block.functions, "test_i64"),
           "test_i64 exists in fn block");
  });

  TEST_PARSER_ERROR("var redef 01", "struct a {i32 b };\n"
                                    "var i32 a = 2;\n",
                    "Variable name 'a' in use by a type, previously defined at "
                    "memory:string:1:1",
                    {});

  TEST_PARSER_ERROR(
      "var redef 01", "var i32 a = 1;\n"
                      "var i32 a = 2;\n",
      "Variable 'a' redefinition, previously defined at memory:string:1:1", {});

  TEST_PARSER_ERROR(
      "var redef 02", "var i32 c = 1;\n"
                      "{global i32 c = 2;}\n",
      "Variable 'c' redefinition, previously defined at memory:string:1:1", {});

  TEST_PARSER_ERROR(
      "var redef 03", "var i32 b = 1;\n"
                      "fn x() { global i32 b = 2; }\n",
      "Variable 'b' redefinition, previously defined at memory:string:1:1", {});

  TEST_PARSER_ERROR("var redef 04", "var i32 b = 1;\n"
                                    "fn b() {}\n",
                    "Function name 'b' in use by a variable, previously "
                    "defined at memory:string:1:1",
                    {});

  TEST_PARSER_ERROR("var redef 05", "fn b() {}\n"
                                    "var i32 b = 1;\n",
                    "Variable name 'b' in use by a type, previously defined at "
                    "memory:string:1:1",
                    {});

  TEST_PARSER_ERROR("var redef 06", "struct b {i8 x};\n"
                                    "fn b() {}\n",
                    "Function name 'b' in use by a type, previously defined at "
                    "memory:string:1:1",
                    {});

  return 0;
}
