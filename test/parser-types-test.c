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

void test_parser_type(fl_ast_t* root, size_t typeid) {
  fl_ast_t* fbody;
  fl_ast_t* var_type;

  ASSERT(root != 0, "string literal found!");

  fbody = *(root->program.body->block.body);
  ASSERT(fbody->type == FL_AST_DTOR_VAR, "first in body: FL_AST_DTOR_VAR");

  var_type = fbody->var.type;
  ASSERT(var_type != 0, "dtor has a type");
  ASSERTE(var_type->type, FL_AST_TYPE, "%d != %d", "dtor type is FL_AST_TYPE");
  ASSERTE(var_type->ty.id, typeid, "%zu != %zu", "typeid ?");
}

// TODO review if ";" is required
TASK_IMPL(parser_types) {
  fl_ast_t* root;
  fl_ast_t* ast;

  root = fl_parse_utf8("var bool hello;");
  test_parser_type(root, 2);
  fl_ast_delete(root);

  root = fl_parse_utf8("var i8 hello;");
  test_parser_type(root, 3);
  fl_ast_delete(root);

  root = fl_parse_utf8("var u8 hello;");
  test_parser_type(root, 4);
  fl_ast_delete(root);

  root = fl_parse_utf8("var i16 hello;");
  test_parser_type(root, 5);
  fl_ast_delete(root);

  root = fl_parse_utf8("var i32 hello;");
  test_parser_type(root, 7);
  fl_ast_delete(root);

  root = fl_parse_utf8("var i64 hello;");
  test_parser_type(root, 9);
  fl_ast_delete(root);

  root = fl_parse_utf8("var f32 hello;");
  test_parser_type(root, 11);
  fl_ast_delete(root);

  root = fl_parse_utf8("var ptr<f32> hello;");
  test_parser_type(root, 13);
  fl_ast_delete(root);

  // unique echeck!
  root = fl_parse_utf8("var ptr<f32> hello;");
  test_parser_type(root, 13);
  fl_ast_delete(root);

  return 0;
}
