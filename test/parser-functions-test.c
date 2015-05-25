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

// TODO review if ";" is required
TASK_IMPL(parser_functions) {
  fl_ast_t* root;
  fl_ast_t* ast;

  root = fl_parse_utf8("fn x() {}");
  ast = *(root->program.body->block.body);

  ASSERT(ast != 0, "string literal found!");

  ASSERT(ast->type == FL_AST_DECL_FUNCTION, "FL_AST_DECL_FUNCTION");
  ASSERT(ast->func.id->type == FL_AST_LIT_IDENTIFIER, "FL_AST_LIT_IDENTIFIER");
  ASSERT(ast->func.params == 0, "no args");
  fl_ast_delete(root);

  root = fl_parse_utf8("fn x(yy, zz , mm ,xx) {}");
  ast = *(root->program.body->block.body);

  ASSERT(ast != 0, "string literal found!");

  ASSERT(ast->type == FL_AST_DECL_FUNCTION, "FL_AST_DECL_FUNCTION");
  ASSERT(ast->func.id->type == FL_AST_LIT_IDENTIFIER, "FL_AST_LIT_IDENTIFIER");
  ASSERT(ast->func.params != 0, "no args");
  fl_ast_delete(root);

  root = fl_parse_utf8("fn {}");

  ASSERT(root->type == FL_AST_ERROR, "error found");
  ASSERT(strcmp(root->err.str, "cannot parse function identifier") == 0, "error found");

  fl_ast_delete(root);


  root = fl_parse_utf8("fn hell ({}");

  ASSERT(root->type == FL_AST_ERROR, "error found");
  ASSERT(strcmp(root->err.str, "expected ')'") == 0, "error found");

  fl_ast_delete(root);

  root = fl_parse_utf8("fn x() { fn;}");

  ASSERT(root->type == FL_AST_ERROR, "error found");
  ASSERT(strcmp(root->err.str, "cannot parse function identifier") == 0, "error found");

  fl_ast_delete(root);


  return 0;
}
