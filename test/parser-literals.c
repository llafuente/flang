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
TASK_IMPL(parser_literals) {
  log_debug_level = 0;

  TEST_PARSER_OK("string literals 01", "\"hello:\\\"w\'orld\"", {
    ASSERT(body[0]->type == FL_AST_LIT_STRING, "FL_AST_LIT_STRING");
  });

  TEST_PARSER_OK("string literals 02", "'hello:\"wo\\\'rld'", {
    ASSERT(body[0]->type == FL_AST_LIT_STRING, "FL_AST_LIT_STRING");
  });

  TEST_PARSER_OK("null literals 01", "null", {
    ASSERT(body[0]->type == FL_AST_LIT_NULL, "FL_AST_LIT_NULL");
  });

  TEST_PARSER_OK("null literals 02", "nil", {
    ASSERT(body[0]->type == FL_AST_LIT_NULL, "FL_AST_LIT_NULL");
  });

  TEST_PARSER_OK("bool literals 01", "true", {
    ASSERT(body[0]->type == FL_AST_LIT_BOOLEAN, "FL_AST_LIT_BOOLEAN");
    ASSERT(body[0]->boolean.value == true, "value = true");
  });

  TEST_PARSER_OK("bool literals 01", "false", {
    ASSERT(body[0]->type == FL_AST_LIT_BOOLEAN, "FL_AST_LIT_BOOLEAN");
    ASSERT(body[0]->boolean.value == false, "value = false");
  });

  TEST_PARSER_OK("numeric literals 01", "1567", {
    ASSERT(body[0]->type == FL_AST_LIT_NUMERIC, "FL_AST_LIT_NUMERIC");
    ASSERT(body[0]->numeric.value == 1567, "value = 1567");
  });

  TEST_PARSER_OK("numeric literals 02", "1e1", {
    ASSERT(body[0]->type == FL_AST_LIT_NUMERIC, "FL_AST_LIT_NUMERIC");
    ASSERT(body[0]->numeric.value == 10, "value = 1567");
  });

  TEST_PARSER_OK("numeric literals 03", "0xff", {
    ASSERT(body[0]->type == FL_AST_LIT_NUMERIC, "FL_AST_LIT_NUMERIC");
    ASSERT(body[0]->numeric.value == 0xff, "value = 0xff");
  });

  TEST_PARSER_OK("numeric literals 04", "5.2", {
    ASSERT(body[0]->type == FL_AST_LIT_NUMERIC, "FL_AST_LIT_NUMERIC");
    ASSERT(body[0]->numeric.value == 5.2, "value = 5.2");
  });

  // TODO binary 0b000000001
  // TODO octal 0o777

  TEST_PARSER_OK("id literals 03", "var wtf; wtf;", {
    ASSERT(body[1]->type == FL_AST_LIT_IDENTIFIER, "FL_AST_LIT_IDENTIFIER");
    ASSERT(strcmp(body[1]->identifier.string->value, "wtf") == 0,
           "identifier = wtf");
    // st_delete(&body[0]->identifier.string);
  });

  return 0;
}
