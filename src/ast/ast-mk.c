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

ast_t* ast_mk_root() {
  printf("ast_mk_root");
  ast_t* node = ast_new();
  node->type = FL_AST_PROGRAM;

  return node;
}

ast_t* ast_mk_list() {
  printf("ast_mk_list");
  ast_t* node = ast_new();
  node->type = FL_AST_LIST;
  node->list.count = 0;
  node->list.elements = calloc(100, sizeof(ast_t*));

  return node;
}

ast_t* ast_mk_list_push(ast_t* list, ast_t* node) {
  printf("ast_mk_list_push");
  node->list.elements[list->list.count++] = node;

  return list;
}

ast_t* ast_mk_block() {
  printf("ast_mk_block");
  ast_t* node = ast_new();
  node->type = FL_AST_BLOCK;

  return node;
}

ast_t* ast_mk_lit_id(char* str, bool resolve) {
  printf("ast_mk_lit_id");
  ast_t* node = ast_new();
  node->type = FL_AST_LIT_IDENTIFIER;
  node->identifier.string = st_newc(str, st_enc_utf8);
  node->identifier.resolve = resolve;

  return node;
}

ast_t* ast_mk_lit_null() {
  printf("ast_mk_lit_null");
  ast_t* node = ast_new();
  node->type = FL_AST_LIT_NULL;

  return node;
}

ast_t* ast_mk_lit_string(char* str, bool interpolate) {
  printf("ast_mk_lit_string");
  ast_t* node = ast_new();
  node->type = FL_AST_LIT_STRING;
  node->string.value = st_newc(str, st_enc_utf8);
  node->string.quoted = interpolate; // TODO rename

  return node;
}

ast_t* ast_mk_lit_boolean(bool value) {
  printf("ast_mk_lit_boolean");
  ast_t* node = ast_new();
  node->type = FL_AST_LIT_BOOLEAN;
  node->boolean.value = value;

  return node;
}

ast_t* ast_mk_return(ast_t* argument) {
  printf("ast_mk_return");
  ast_t* node = ast_new();
  node->type = FL_AST_STMT_RETURN;
  node->ret.argument = argument;

  return node;
}

ast_t* ast_mk_var_decl(ast_t* type, ast_t* id) {
  ast_t* node = ast_new();
  node->type = FL_AST_DTOR_VAR;
  node->var.id = id;
  node->var.type = type;
  // node->var.alloca = 0;
  // node->ty_id = 0;

  return node;
}

ast_t* ast_mk_fn_decl(ast_t* id, ast_t* ret_type, ast_t* params, ast_t* body) {
  ast_t* node = ast_new();
  node->type = FL_AST_DECL_FUNCTION;
  node->func.id = id;
  node->func.ret_type = ret_type;
  node->func.params = params;
  node->func.body = body;

  return node;
}

ast_t* ast_mk_numeric(char* text) {
  ast_t* node = ast_new();
  node->type = FL_AST_LIT_NUMERIC;
  node->numeric.d_value = 10;
  node->numeric.li_value = 11;
  node->numeric.lui_value = 12;

  return node;
}

ast_t* ast_mk_binop(ast_t* left, int op, ast_t* right) {
  ast_t* node = ast_new();
  node->type = FL_AST_LIT_NUMERIC;
  node->binop.left = left;
  node->binop.right = right;
  node->binop.operator= op;

  return node;
}

ast_t* ast_mk_assignament(ast_t* left, int op, ast_t* right) {
  ast_t* node = ast_new();
  node->type = FL_AST_EXPR_ASSIGNAMENT;
  node->assignament.left = left;
  node->assignament.right = right;
  node->assignament.operator= op;

  return node;
}

ast_t* ast_mk_call_expr(ast_t* callee, ast_t* arguments) {
  ast_t* node = ast_new();

  node->type = FL_AST_EXPR_CALL;
  node->call.callee = callee;
  node->call.arguments = arguments;
  node->call.narguments = arguments->list.count;

  return node;
}
