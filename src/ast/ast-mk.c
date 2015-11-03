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

// TODO redefinition atm...
// ast_t* ast_new() { return (ast_t*)calloc(1, sizeof(ast_t)); }

ast_t* ast_mk_program(ast_t* block) {
  ast_t* node = ast_new();
  node->type = FL_AST_PROGRAM;
  node->program.body = block;

  return node;
}
ast_t* ast_mk_list() {
  printf("ast_mk_list\n");
  ast_t* node = ast_new();
  node->type = FL_AST_LIST;
  node->list.count = 0;
  node->list.elements = calloc(300, sizeof(ast_t*));

  return node;
}

ast_t* ast_mk_list_push(ast_t* list, ast_t* node) {
  printf("ast_mk_list_push [%p]\n", list);
  list->list.elements[list->list.count++] = node;

  return list;
}

ast_t* ast_mk_block(ast_t* body) {
  printf("ast_mk_block\n");
  ast_t* node = ast_new();
  node->type = FL_AST_BLOCK;
  node->block.body = body;

  return node;
}

ast_t* ast_mk_lit_id(string* str, bool resolve) {
  printf("ast_mk_lit_id '%s'\n", str->value);

  ast_t* node = ast_new();
  node->type = FL_AST_LIT_IDENTIFIER;
  node->identifier.string = str;
  node->identifier.resolve = resolve;

  return node;
}

ast_t* ast_mk_lit_null() {
  printf("ast_mk_lit_null\n");
  ast_t* node = ast_new();
  node->type = FL_AST_LIT_NULL;

  return node;
}

ast_t* ast_mk_lit_string(char* str, bool interpolate) {
  printf("ast_mk_lit_string\n");
  ast_t* node = ast_new();
  node->type = FL_AST_LIT_STRING;
  node->string.value = st_newc(str, st_enc_utf8);
  node->string.quoted = interpolate; // TODO rename

  return node;
}

ast_t* ast_mk_lit_boolean(bool value) {
  printf("ast_mk_lit_boolean\n");
  ast_t* node = ast_new();
  node->type = FL_AST_LIT_BOOLEAN;
  node->boolean.value = value;

  return node;
}
// TODO do the parsing magic!
ast_t* ast_mk_lit_integer(char* text) {
  ast_t* node = ast_new();
  node->type = FL_AST_LIT_NUMERIC;
  node->numeric.d_value = 99;
  node->numeric.li_value = 99;
  node->numeric.lui_value = 99;

  return node;
}
// TODO do the parsing magic!
ast_t* ast_mk_lit_float(char* text) {
  ast_t* node = ast_new();
  node->type = FL_AST_LIT_NUMERIC;
  node->numeric.d_value = 101;
  node->numeric.li_value = 101;
  node->numeric.lui_value = 101;

  return node;
}

ast_t* ast_mk_return(ast_t* argument) {
  printf("ast_mk_return\n");
  ast_t* node = ast_new();
  node->type = FL_AST_STMT_RETURN;
  node->ret.argument = argument;

  return node;
}

ast_t* ast_mk_break(ast_t* argument) {
  printf("ast_mk_break\n");
  ast_t* node = ast_new();
  node->type = FL_AST_STMT_BREAK;
  node->brk.argument = argument;

  return node;
}

ast_t* ast_mk_continue(ast_t* argument) {
  printf("ast_mk_break\n");
  ast_t* node = ast_new();
  node->type = FL_AST_STMT_CONTINUE;
  node->cont.argument = argument;

  return node;
}

ast_t* ast_mk_var_decl(ast_t* type, ast_t* id) {
  printf("ast_mk_var_decl\n");
  ast_t* node = ast_new();
  node->type = FL_AST_DTOR_VAR;
  node->var.id = id;
  node->var.type = type;
  // node->var.alloca = 0;
  // node->ty_id = 0;

  return node;
}

ast_t* ast_mk_fn_decl(ast_t* id, ast_t* params, ast_t* ret_type, ast_t* body) {
  printf("ast_mk_fn_decl\n");
  ast_t* node = ast_new();
  node->type = FL_AST_DECL_FUNCTION;
  node->func.id = id;
  node->func.uid =
      st_clone(id->identifier.string); // TODO this could be removed
  node->func.ret_type = ret_type;
  node->func.params = params;
  node->func.body = body;

  return node;
}

ast_t* ast_mk_fn_param(ast_t* id, ast_t* type, ast_t* def) {
  printf("ast_mk_fn_param\n");
  ast_t* node = ast_new();
  node->type = FL_AST_PARAMETER;

  node->param.id = id;
  node->param.type = type;
  node->param.def = def;

  // TODO assertions

  return node;
}

ast_t* ast_mk_numeric(char* text) {
  printf("ast_mk_numeric\n");
  ast_t* node = ast_new();
  node->type = FL_AST_LIT_NUMERIC;
  node->numeric.d_value = 10;
  node->numeric.li_value = 11;
  node->numeric.lui_value = 12;

  return node;
}

ast_t* ast_mk_binop(ast_t* left, int op, ast_t* right) {
  printf("ast_mk_binop\n");
  ast_t* node = ast_new();
  node->type = FL_AST_EXPR_BINOP;
  node->binop.left = left;
  node->binop.right = right;
  node->binop.operator= op;

  return node;
}

ast_t* ast_mk_assignament(ast_t* left, int op, ast_t* right) {
  printf("ast_mk_assignament\n");
  ast_t* node = ast_new();
  node->type = FL_AST_EXPR_ASSIGNAMENT;
  node->assignament.left = left;
  node->assignament.right = right;
  node->assignament.operator= op;

  return node;
}

ast_t* ast_mk_call_expr(ast_t* callee, ast_t* arguments) {
  printf("ast_mk_call_expr\n");
  ast_t* node = ast_new();

  node->type = FL_AST_EXPR_CALL;
  node->call.callee = callee;
  node->call.arguments = arguments;
  node->call.narguments = arguments ? arguments->list.count : 0;

  return node;
}

ast_t* ast_mk_type(string* id, ast_t* child) {
  printf("ast_mk_type\n");
  ast_t* node = ast_new();

  node->type = FL_AST_TYPE;
  node->ty.id = id ? ast_mk_lit_id(id, false) : 0;
  node->ty.child = child;

  return node;
}

ast_t* ast_mk_comment(string* text) {
  printf("ast_mk_comment\n");
  ast_t* node = ast_new();

  node->type = FL_AST_STMT_COMMENT;
  node->comment.text = text;

  return node;
}

ast_t* ast_mk_lunary(ast_t* element, int operator) {
  printf("ast_mk_lunary\n");
  ast_t* node = ast_new();

  node->type = FL_AST_EXPR_LUNARY;
  node->lunary.element = element;
  node->lunary.operator= operator;

  return node;
}

ast_t* ast_mk_if(ast_t* test, ast_t* block, ast_t* alternate) {
  printf("ast_mk_if\n");
  ast_t* node = ast_new();

  node->type = FL_AST_STMT_IF;
  node->if_stmt.test = test;
  node->if_stmt.block = block;
  node->if_stmt.alternate = alternate;

  return node;
}

ast_t* ast_mk_loop(ast_t* init, ast_t* pre_cond, ast_t* update, ast_t* block,
                   ast_t* post_cond) {
  printf("ast_mk_loop\n");
  ast_t* node = ast_new();

  node->type = FL_AST_STMT_LOOP;

  node->loop.init = init;
  node->loop.pre_cond = pre_cond;
  node->loop.update = update;
  node->loop.block = block;
  node->loop.post_cond = post_cond;
  // TODO node->loop.type = type;

  return node;
}

ast_t* ast_mk_struct_decl(ast_t* id, ast_t* fields) {
  printf("ast_mk_struct_decl\n");
  ast_t* node = ast_new();

  node->type = FL_AST_DECL_STRUCT;

  node->structure.id = id;
  node->structure.fields = fields;

  return node;
}

ast_t* ast_mk_struct_decl_field(ast_t* id, ast_t* type) {
  printf("ast_mk_struct_decl_field\n");
  ast_t* node = ast_new();

  node->type = FL_AST_DECL_STRUCT_FIELD;

  node->field.id = id;
  node->field.type = type;

  return node;
}

ast_t* ast_mk_member(ast_t* left, ast_t* property, bool expression) {
  printf("ast_mk_member\n");
  ast_t* node = ast_new();

  node->type = FL_AST_EXPR_MEMBER;

  node->member.left = left;
  node->member.property = property;
  node->member.expression = expression;

  return node;
}
