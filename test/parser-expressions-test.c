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
TASK_IMPL(parser_expressions) {
  fl_ast_t* root;
  fl_ast_t* ast;
  root = fl_parse_utf8("1+2");

  // fl_ast_traverse(root, fl_ast_debug_cb, 0, 0);

  ast = *(root->program.body);
  ASSERT(ast != 0, "string literal found!");
  ASSERT(ast->type == FL_AST_EXPR_BINOP, "FL_AST_EXPR_BINOP");
  ASSERT(ast->binop.left->type == FL_AST_LIT_NUMERIC, "FL_AST_LIT_NUMERIC");
  ASSERT(ast->binop.right->type == FL_AST_LIT_NUMERIC, "FL_AST_LIT_NUMERIC");
  fl_ast_delete(root);

  root = fl_parse_utf8("1+2+3");

  fl_ast_traverse(root, fl_ast_debug_cb, 0, 0);

  ast = *(root->program.body);
  ASSERT(ast != 0, "string literal found!");
  ASSERT(ast->type == FL_AST_EXPR_BINOP, "FL_AST_EXPR_BINOP");
  ASSERT(ast->binop.left->type == FL_AST_LIT_NUMERIC, "FL_AST_LIT_NUMERIC");
  ASSERT(ast->binop.right->type == FL_AST_EXPR_BINOP, "FL_AST_EXPR_BINOP");
  ASSERT(ast->binop.right->binop.left->type == FL_AST_LIT_NUMERIC,
         "FL_AST_LIT_NUMERIC");
  ASSERT(ast->binop.right->binop.right->type == FL_AST_LIT_NUMERIC,
         "FL_AST_LIT_NUMERIC");
  fl_ast_delete(root);

  root = fl_parse_utf8("1*2");

  fl_ast_traverse(root, fl_ast_debug_cb, 0, 0);

  ast = *(root->program.body);
  ASSERT(ast != 0, "string literal found!");
  ASSERT(ast->type == FL_AST_EXPR_BINOP, "FL_AST_EXPR_BINOP");
  ASSERT(ast->binop.left->type == FL_AST_LIT_NUMERIC, "FL_AST_LIT_NUMERIC");
  ASSERT(ast->binop.operator== FL_TK_ASTERISK, "FL_TK_ASTERISK");
  ASSERT(ast->binop.right->type == FL_AST_LIT_NUMERIC, "FL_AST_LIT_NUMERIC");
  fl_ast_delete(root);

  root = fl_parse_utf8("1*2+3"); // priotity test

  fl_ast_traverse(root, fl_ast_debug_cb, 0, 0);

  ast = *(root->program.body);
  ASSERT(ast != 0, "string literal found!");
  ASSERT(ast->type == FL_AST_EXPR_BINOP, "FL_AST_EXPR_BINOP");

  ASSERT(ast->binop.left->binop.left->type == FL_AST_LIT_NUMERIC,
         "FL_AST_LIT_NUMERIC");
  ASSERT(ast->binop.left->binop.operator== FL_TK_ASTERISK,
         "left FL_TK_ASTERISK");
  ASSERT(ast->binop.left->binop.right->type == FL_AST_LIT_NUMERIC,
         "FL_AST_LIT_NUMERIC");

  ASSERT(ast->binop.operator== FL_TK_PLUS, "FL_TK_PLUS");
  ASSERT(ast->binop.right->type == FL_AST_LIT_NUMERIC, "FL_AST_LIT_NUMERIC");

  fl_ast_delete(root);

  root = fl_parse_utf8("-2"); // unary

  fl_ast_traverse(root, fl_ast_debug_cb, 0, 0);

  ast = *(root->program.body);
  ASSERT(ast != 0, "ast found!");
  ASSERT(ast->type == FL_AST_EXPR_LUNARY, "ast is FL_AST_EXPR_LUNARY");

  ASSERT(ast->lunary.element->type == FL_AST_LIT_NUMERIC, "FL_AST_LIT_NUMERIC");
  ASSERT(ast->lunary.operator== FL_TK_MINUS, "operator FL_TK_MINUS");

  fl_ast_delete(root);

  root = fl_parse_utf8("xxx++"); // unary

  fl_ast_traverse(root, fl_ast_debug_cb, 0, 0);

  ast = *(root->program.body);
  ASSERT(ast != 0, "ast found!");
  ASSERT(ast->type == FL_AST_EXPR_RUNARY, "ast is FL_AST_EXPR_RUNARY");

  ASSERT(ast->lunary.element->type == FL_AST_LIT_IDENTIFIER,
         "FL_AST_LIT_IDENTIFIER");
  ASSERT(ast->lunary.operator== FL_TK_PLUS2, "operator FL_TK_PLUS2");

  fl_ast_delete(root);

  root = fl_parse_utf8("a=b"); // unary

  fl_ast_traverse(root, fl_ast_debug_cb, 0, 0);

  ast = *(root->program.body);
  ASSERT(ast != 0, "ast found!");
  ASSERT(ast->type == FL_AST_EXPR_ASSIGNAMENT,
         "ast is FL_AST_EXPR_ASSIGNAMENT");
  ASSERT(ast->assignament.operator== FL_TK_EQUAL, "operator: FL_TK_EQUAL");

  ASSERT(ast->assignament.left->type == FL_AST_LIT_IDENTIFIER,
         "left: FL_AST_LIT_IDENTIFIER");
  ASSERT(ast->assignament.right->type == FL_AST_LIT_IDENTIFIER,
         "right: FL_AST_LIT_IDENTIFIER");

  fl_ast_delete(root);

  root = fl_parse_utf8("a =b"); // unary

  fl_ast_traverse(root, fl_ast_debug_cb, 0, 0);

  ast = *(root->program.body);
  ASSERT(ast != 0, "ast found!");
  fl_ast_delete(root);

  return 0;
}
