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
  fl_ast_t* root;
  fl_ast_t* ast;
  fl_ast_t* body;

  root = fl_parse_utf8("var i64 c; c = 1 + 2;");

  ast = *(root->program.body->block.body);
  ASSERT(ast != 0, "ast parsed ok");

  fl_codegen(root, "test");

  fl_ast_delete(root);

  root = fl_parse_utf8("var string hello;");

  ast = *(root->program.body->block.body);
  ASSERT(ast != 0, "ast parsed ok");

  fl_codegen(root, "test");

  fl_ast_delete(root);

  root = fl_parse_utf8("var i64 c; var i64 d; d=1+2; c = 1;");

  fl_ast_traverse(root, fl_ast_debug_cb, 0, 0, 0);

  ast = *(root->program.body->block.body);
  ASSERT(ast != 0, "ast parsed ok");

  fl_codegen(root, "test");

  fl_ast_delete(root);

  root = fl_parse_utf8("printf(\"%f\", 1 + 2);");
  fl_parse_core(root);
  CHK_BODY(root)
  fl_codegen(root, "test");
  fl_ast_delete(root);

  return 0;
}
