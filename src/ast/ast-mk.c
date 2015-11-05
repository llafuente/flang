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
#include <limits.h>
#include <errno.h>

// TODO redefinition atm...
// ast_t* ast_new() { return (ast_t*)calloc(1, sizeof(ast_t)); }

void ast_position(ast_t* target, YYLTYPE start, YYLTYPE end) {
  target->first_line = start.first_line;
  target->first_column = start.first_column;
  target->last_line = end.last_line;
  target->last_column = end.last_column;
}

ast_t* ast_mk_program(ast_t* block) {
  ast_t* node = ast_new();
  node->type = FL_AST_PROGRAM;

  node->program.body = block;

  return node;
}
ast_t* ast_mk_list() {
  // printf("ast_mk_list\n");
  ast_t* node = ast_new();
  node->type = FL_AST_LIST;

  node->list.count = 0;
  node->list.elements = calloc(300, sizeof(ast_t*));

  return node;
}

ast_t* ast_mk_list_push(ast_t* list, ast_t* node) {
  // printf("ast_mk_list_push [%p]\n", list);
  list->list.elements[list->list.count++] = node;
  return list;
}

ast_t* ast_mk_list_insert(ast_t* list, ast_t* node, size_t idx) {
  // printf("ast_mk_list_push [%p]\n", list);
  size_t count = list->list.count;
  assert(count > idx);

  memmove(list->list.elements + idx + 1, list->list.elements + idx,
          sizeof(ast_t*) * count - idx);

  list->list.elements[idx] = node;
  ++list->list.count;

  return list;
}
ast_t* ast_mk_insert_before(ast_t* list, ast_t* search_item,
                            ast_t* insert_item) {
  size_t idx = 0;
  while (list->list.elements[idx] == search_item) {
    ++idx;
    if (idx > list->list.count) {
      fprintf(stderr, "ast_mk_insert_before not found 'search_item'\n");
      exit(2);
    }
  }

  ast_mk_list_insert(list, insert_item, idx);
  return list;
}

ast_t* ast_mk_block(ast_t* body) {
  // printf("ast_mk_block\n");
  ast_t* node = ast_new();
  node->type = FL_AST_BLOCK;

  node->block.body = body;

  return node;
}

ast_t* ast_mk_lit_id(string* str, bool resolve) {
  // printf("ast_mk_lit_id '%s'\n", str->value);

  ast_t* node = ast_new();
  node->type = FL_AST_LIT_IDENTIFIER;

  node->identifier.string = str;
  node->identifier.resolve = resolve;

  return node;
}

ast_t* ast_mk_lit_null() {
  // printf("ast_mk_lit_null\n");
  ast_t* node = ast_new();
  node->type = FL_AST_LIT_NULL;

  return node;
}

ast_t* ast_mk_lit_string(char* str, bool interpolate) {
  // printf("ast_mk_lit_string\n");
  ast_t* node = ast_new();
  node->type = FL_AST_LIT_STRING;
  node->ty_id = TS_CSTR;

  node->string.value = st_newc(str, st_enc_utf8);
  node->string.quoted = interpolate; // TODO rename

  return node;
}

ast_t* ast_mk_lit_boolean(bool value) {
  // printf("ast_mk_lit_boolean\n");
  ast_t* node = ast_new();
  node->type = FL_AST_LIT_BOOLEAN;
  node->ty_id = TS_BOOL;

  node->boolean.value = value;

  return node;
}
// TODO do the parsing magic!
ast_t* ast_mk_lit_integer(char* text) {
  ast_t* node = ast_new();
  node->type = FL_AST_LIT_INTEGER;

  char* end = text + strlen(text);
  long long val = strtol(text, &end, 10);

  log_verbose("read [%lld] [%d == %d]", val, errno, ERANGE);

  if (errno == ERANGE ||
      (errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)) ||
      (errno != 0 && val == 0)) {
    log_verbose("ERANGE: try to read long unsigned int");
    node->integer.unsigned_value = strtoul(text, &end, 10);
    if (errno == ERANGE) {
      log_verbose("ERANGE: double");
      // TODO ??
    } else {
      node->ty_id = TS_U64;
    }
  } else {
    node->integer.signed_value = val;
    node->ty_id = TS_I64;
  }

  return node;
}
// TODO do the parsing magic!
ast_t* ast_mk_lit_float(char* text) {
  ast_t* node = ast_new();
  node->type = FL_AST_LIT_FLOAT;

  char* end = text + strlen(text);
  double result = strtod(text, &end);
  if (errno) {
    if ((result == HUGE_VAL || result == -HUGE_VAL) && errno == ERANGE) {
      // fprintf(stderr, "ERROR! overflow\n");
    } else if (errno == ERANGE) {
      // fprintf(stderr, "ERROR! underflow\n");
    }
  }
  node->decimal.value = result;
  node->ty_id = TS_F64;

  return node;
}

ast_t* ast_mk_return(ast_t* argument) {
  // printf("ast_mk_return\n");
  ast_t* node = ast_new();
  node->type = FL_AST_STMT_RETURN;

  node->ret.argument = argument;

  return node;
}

ast_t* ast_mk_break(ast_t* argument) {
  // printf("ast_mk_break\n");
  ast_t* node = ast_new();
  node->type = FL_AST_STMT_BREAK;

  node->brk.argument = argument;

  return node;
}

ast_t* ast_mk_continue(ast_t* argument) {
  // printf("ast_mk_break\n");
  ast_t* node = ast_new();
  node->type = FL_AST_STMT_CONTINUE;

  node->cont.argument = argument;

  return node;
}

ast_t* ast_mk_var_decl(ast_t* type, ast_t* id) {
  // printf("ast_mk_var_decl\n");
  ast_t* node = ast_new();
  node->type = FL_AST_DTOR_VAR;

  node->var.id = id;
  node->var.type = type;

  return node;
}

ast_t* ast_mk_fn_decl(ast_t* id, ast_t* params, ast_t* ret_type, ast_t* body) {
  // printf("ast_mk_fn_decl\n");
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
  // printf("ast_mk_fn_param\n");
  ast_t* node = ast_new();
  node->type = FL_AST_PARAMETER;

  node->param.id = id;
  node->param.type = type;
  node->param.def = def;

  // TODO assertions

  return node;
}

ast_t* ast_mk_binop(ast_t* left, int op, ast_t* right) {
  // printf("ast_mk_binop\n");
  ast_t* node = ast_new();
  node->type = FL_AST_EXPR_BINOP;

  node->binop.left = left;
  node->binop.right = right;
  node->binop.operator= op;

  return node;
}

ast_t* ast_mk_assignament(ast_t* left, int op, ast_t* right) {
  // printf("ast_mk_assignament\n");
  ast_t* node = ast_new();
  node->type = FL_AST_EXPR_ASSIGNAMENT;

  node->assignament.left = left;
  node->assignament.right = right;
  node->assignament.operator= op;

  return node;
}

ast_t* ast_mk_call_expr(ast_t* callee, ast_t* arguments) {
  // printf("ast_mk_call_expr\n");
  ast_t* node = ast_new();
  node->type = FL_AST_EXPR_CALL;

  node->call.callee = callee;
  node->call.arguments = arguments;
  node->call.narguments = arguments ? arguments->list.count : 0;

  return node;
}

ast_t* ast_mk_type(string* id, ast_t* child) {
  // printf("ast_mk_type\n");
  ast_t* node = ast_new();
  node->type = FL_AST_TYPE;

  node->ty.id = id ? ast_mk_lit_id(id, false) : 0;
  node->ty.child = child;

  return node;
}

ast_t* ast_mk_comment(string* text) {
  // printf("ast_mk_comment\n");
  ast_t* node = ast_new();
  node->type = FL_AST_STMT_COMMENT;

  node->comment.text = text;

  return node;
}

ast_t* ast_mk_lunary(ast_t* element, int operator) {
  // printf("ast_mk_lunary\n");
  ast_t* node = ast_new();
  node->type = FL_AST_EXPR_LUNARY;

  node->lunary.element = element;
  node->lunary.operator= operator;

  return node;
}

ast_t* ast_mk_if(ast_t* test, ast_t* block, ast_t* alternate) {
  // printf("ast_mk_if\n");
  ast_t* node = ast_new();
  node->type = FL_AST_STMT_IF;

  node->if_stmt.test = test;
  node->if_stmt.block = block;
  node->if_stmt.alternate = alternate;

  return node;
}

ast_t* ast_mk_loop(ast_t* init, ast_t* pre_cond, ast_t* update, ast_t* block,
                   ast_t* post_cond) {
  // printf("ast_mk_loop\n");
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
  // printf("ast_mk_struct_decl\n");
  ast_t* node = ast_new();
  node->type = FL_AST_DECL_STRUCT;

  id->identifier.resolve = false;
  node->structure.id = id;
  node->structure.fields = fields;

  return node;
}

ast_t* ast_mk_struct_decl_field(ast_t* id, ast_t* type) {
  // printf("ast_mk_struct_decl_field\n");
  ast_t* node = ast_new();
  node->type = FL_AST_DECL_STRUCT_FIELD;

  id->identifier.resolve = false;
  node->field.id = id;
  node->field.type = type;

  return node;
}

ast_t* ast_mk_member(ast_t* left, ast_t* property, bool expression) {
  // printf("ast_mk_member\n");
  ast_t* node = ast_new();
  node->type = FL_AST_EXPR_MEMBER;

  if (property->type == FL_AST_LIT_IDENTIFIER) {
    property->identifier.resolve = false;
  }

  node->member.left = left;
  node->member.property = property;
  node->member.expression = expression;

  return node;
}

ast_t* ast_mk_cast(ast_t* type, ast_t* element) {
  // printf("ast_mk_cast\n");
  ast_t* node = ast_new();
  node->type = FL_AST_CAST;

  node->cast.type = type;
  node->cast.element = element;

  return node;
}

ast_t* ast_mk_import(ast_t* string_lit) {
  // printf("ast_mk_import\n");
  ast_t* node = ast_new();
  node->type = FL_AST_IMPORT;

  node->import.path = string_lit;

  return node;
}
