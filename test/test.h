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

#include "flang/common.h"
#include "flang/flang.h"
#include "flang/libast.h"
#include "flang/libparser.h"
#include "flang/libts.h"
#include "flang/debug.h"
#include "flang/libcg.h"

#define STR(val) #val

// last core typeid + 1
#define TEST_TYPEID 16

#define STRING_MATCH(a, b)                                                     \
  if (strcmp(a, b) != 0) {                                                     \
    fprintf(stderr, "expected: %s\nfound:%s\n", b, a);                         \
    ASSERT(false, "error message match");                                      \
  }

#define CHK_BODY(root)                                                         \
  ASSERT(root != 0, "root is not null");                                       \
  ASSERT(root->type == AST_PROGRAM, "root is a program");                      \
  ASSERT(root->program.body->type == AST_BLOCK, "program has body");           \
  ASSERT(root->program.body->block.body->list.count > 0, "body has "           \
                                                         "statements");

#define CHK_GET_BODY(root, target)                                             \
  CHK_BODY(root);                                                              \
  target = root->program.body->block.body;

#define CHK_ERROR(root, target, msg)                                           \
  ASSERT(root != 0, "root is not null");                                       \
  ASSERT(root->type == AST_PROGRAM, "root is a program");                      \
  target = root->program.body;                                                 \
  ASSERT(target->type == AST_ERROR, "body is an error");                       \
  STRING_MATCH(target->err.message->value, msg);

#define CHK_ERROR_RANGE(target, sc, sl, ec, el)
/*
  ASSERTE(target->token_start->start.column, sc, "%zu != %d", "start column"); \
  ASSERTE(target->token_start->start.line, sl, "%zu != %d", "start line");     \
  ASSERTE(target->token_end->end.column, ec, "%zu != %d", "end column");       \
  ASSERTE(target->token_end->end.line, el, "%zu != %d", "end line");
*/
#define TEST_PARSER_OK(name, code, code_block)                                 \
  {                                                                            \
    fprintf(stderr, __FILE__ ":" STR(__LINE__) " @ " name "\n");               \
    flang_init();                                                              \
    ast_t* root = psr_str_utf8(code);                                          \
    CHK_BODY(root);                                                            \
    root = typesystem(root);                                                   \
    if (ast_last_error_message) {                                              \
      fprintf(stderr, "unexpected typesystem error");                          \
      exit(1);                                                                 \
    }                                                                          \
    ast_t* mainblock = root->program.body;                                     \
    ast_t** body = root->program.body->block.body->list.elements;              \
    code_block;                                                                \
    flang_exit(root);                                                          \
  }

#define TEST_PARSER_ERROR(name, code, msg, code_block)                         \
  {                                                                            \
    fprintf(stderr, __FILE__ ":" STR(__LINE__) " @ " name "\n");               \
    flang_init();                                                              \
    ast_t* root = psr_str_utf8(code);                                          \
    ASSERT(root != 0, "root is not null");                                     \
    ASSERT(root->type == AST_PROGRAM, "root is a program");                    \
    ast_t* err = root->program.body;                                           \
    ast_mindump(root);                                                         \
    if (err->type == AST_ERROR) {                                              \
      STRING_MATCH(err->err.message->value, msg);                              \
    } else {                                                                   \
      root = typesystem(root);                                                 \
      STRING_MATCH(ast_last_error_message, msg);                               \
    }                                                                          \
    code_block;                                                                \
    flang_exit(root);                                                          \
  }

#define TEST_CODEGEN_OK(name, code, code_block)                                \
  {                                                                            \
    fprintf(stderr, __FILE__ ":" STR(__LINE__) " @ " name "\n");               \
    flang_init();                                                              \
    ast_t* root = psr_str_utf8_main(code);                                     \
    CHK_BODY(root);                                                            \
    root = typesystem(root);                                                   \
    ast_mindump(root);                                                         \
    if (ast_last_error_message) {                                              \
      fprintf(stderr, "unexpected typesystem error");                          \
      exit(1);                                                                 \
    }                                                                          \
    ast_t** body = root->program.body->block.body->list.elements;              \
    fl_codegen(root);                                                          \
    code_block;                                                                \
    flang_exit(root);                                                          \
  }

#define TEST_CODEGEN_OK_NOCORE(name, code, code_block)                         \
  {                                                                            \
    fprintf(stderr, __FILE__ ":" STR(__LINE__) " @ " name "\n");               \
    flang_init();                                                              \
    ast_t* root = psr_str_utf8_main(code);                                     \
    CHK_BODY(root);                                                            \
    ts_register_types(root);                                                   \
    root = ts_pass(root);                                                      \
    ast_mindump(root);                                                         \
    ast_t** body = root->program.body->block.body->list.elements;              \
    fl_codegen(root);                                                          \
    code_block;                                                                \
    flang_exit(root);                                                          \
  }
