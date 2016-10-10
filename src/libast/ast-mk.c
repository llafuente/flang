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

#include "flang/flang.h"
#include "flang/libast.h"
#include "flang/libts.h"
#include "flang/debug.h"
#include "flang/libparser.h"
#include <limits.h>
#include <errno.h>
#include <math.h> // HUGE_VAL

char ast_mk_buffer[64];

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
  node->type = AST_PROGRAM;

  node->program.body = block;

  return node;
}

// TODO maybe string*
ast_t* ast_mk_error(const char* message, char* type) {
  ast_t* node = ast_new();
  node->type = AST_ERROR;

  node->err.message = st_newc(message, st_enc_utf8);
  node->err.type = st_newc(type, st_enc_utf8);

  return node;
}

ast_t* ast_mk_list() {
  // printf("ast_mk_list\n");
  ast_t* node = ast_new();
  node->type = AST_LIST;

  array_newcap((array*)&node->list, 10);
  return node;
}

ast_t* ast_mk_list_push(ast_t* list, ast_t* node) {
  // printf("ast_mk_list_push [%p]\n", list);
  fl_assert(list->type == AST_LIST);

  // list->list.values[list->list.length++] = node;
  array_push((array*)&list->list, (void*)node);
  return list;
}

ast_t* ast_mk_list_pop(ast_t* list) {
  // printf("ast_mk_list_push [%p]\n", list);
  fl_assert(list->type == AST_LIST);

  return (ast_t*)array_pop((array*)&list->list);
}

ast_t* ast_mk_list_insert(ast_t* list, ast_t* node, u64 idx) {
  // printf("ast_mk_list_push [%p]\n", list);
  fl_assert(list->type == AST_LIST);

  array_insert((array*)&list->list, idx, (void*)node);

  return list;
}
ast_t* ast_mk_insert_before(ast_t* list, ast_t* search_item,
                            ast_t* insert_item) {
  fl_assert(list->type == AST_LIST);

  u64 idx = 0;
  while (list->list.values[idx] != search_item) {
    ++idx;
    if (idx > list->list.length) {
      fprintf(stderr, "ast_mk_insert_before not found 'search_item'\n");
      exit(2);
    }
  }

  ast_mk_list_insert(list, insert_item, idx);
  insert_item->parent = list;
  return list;
}

ast_t* ast_mk_block(ast_t* body) {
  // printf("ast_mk_block\n");
  ast_t* node = ast_new();
  node->type = AST_BLOCK;

  node->block.body = body;

  array_new(&node->block.modules);

  node->block.types = pool_new(sizeof(hash_t));
  hash_new(node->block.types, 10);

  node->block.functions = pool_new(sizeof(hash_t));
  hash_new(node->block.functions, 10);

  node->block.variables = pool_new(sizeof(hash_t));
  hash_new(node->block.variables, 10);

  // uids; is allocated by parser.y, only on global

  return node;
}

ast_t* ast_mk_lit_id(string* str, bool resolve) {
  // printf("ast_mk_lit_id '%s'\n", str->value);

  ast_t* node = ast_new();
  node->type = AST_LIT_IDENTIFIER;

  node->identifier.string = str;
  node->identifier.resolve = resolve;

  return node;
}

ast_t* ast_mk_lit_null() {
  // printf("ast_mk_lit_null\n");
  ast_t* node = ast_new();
  node->type = AST_LIT_NULL;

  return node;
}

ast_t* ast_mk_lit_string(char* str, bool interpolate) {
  // printf("ast_mk_lit_string\n");
  ast_t* node = ast_new();
  node->type = AST_LIT_STRING;
  node->ty_id = TS_STRING;
  if (str[0] == '"' || str[0] == '\'') {
    str[strlen(str) - 1] = '\0';
    string* tmp = st_newc(str + 1, st_enc_utf8);
    node->string.value = st_unescape(tmp);
    st_delete(&tmp);
  } else {
    node->string.value = st_newc(str, st_enc_utf8);
  }
  node->string.quoted = interpolate; // TODO rename

  return node;
}

ast_t* ast_mk_lit_string2(string* str, bool interpolate, bool escape) {
  // printf("ast_mk_lit_string\n");
  ast_t* node = ast_new();
  node->type = AST_LIT_STRING;
  node->ty_id = TS_STRING;
  if (escape) {
    node->string.value = st_unescape(str);
  } else {

    node->string.value = str;
  }
  node->string.quoted = interpolate; // TODO rename

  return node;
}

ast_t* ast_mk_lit_boolean(bool value) {
  // printf("ast_mk_lit_boolean\n");
  ast_t* node = ast_new();
  node->type = AST_LIT_BOOLEAN;
  node->ty_id = TS_BOOL;

  node->boolean.value = value;

  return node;
}
// TODO do the parsing magic!
ast_t* ast_mk_lit_integer(char* text) {
  ast_t* node = ast_new();
  node->type = AST_LIT_INTEGER;

  char* end;
  long long val = strtol(text, &end, 10);

  log_silly("read [%lld] errno? [%d == %d]", val, errno, ERANGE);
  if (*end == '\0') {
    // conversion OK!
    node->integer.signed_value = val;
    node->integer.unsigned_value = (long unsigned int)val;
    node->ty_id = TS_I64;
  } else if (errno == ERANGE ||
             (errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)) ||
             (errno != 0 && val == 0)) {
    log_silly("ERANGE: try to read long unsigned int");
    node->integer.unsigned_value = strtoul(text, &end, 10);
    node->integer.signed_value = (long int)node->integer.unsigned_value;
    if (errno == ERANGE) {
      // TODO ??
      log_error("ERANGE: double");
    } else {
      node->ty_id = TS_U64;
    }
  }

  return node;
}
// TODO do the parsing magic!
ast_t* ast_mk_lit_float(char* text) {
  ast_t* node = ast_new();
  node->type = AST_LIT_FLOAT;

  char* end = 0;
  errno = 0;
  float f_res = strtof(text, &end);
  if (errno) {
    int errnum = errno;
    fprintf(stderr, "as double! %d: %s @ '%s'\n", errnum, strerror(errnum),
            text);

    end = 0;
    double d_res = strtod(text, &end);
    if (errno) {
      if ((f_res == HUGE_VAL || f_res == -HUGE_VAL) && errno == ERANGE) {
        fprintf(stderr, "ERROR! overflow\n");
      } else if (errno == ERANGE) {
        fprintf(stderr, "ERROR! underflow\n");
      }
      errnum = errno;
      fprintf(stderr, "%d: %s @ '%s'\n", errnum, strerror(errnum), text);
      // TODO raise syntax-error, no exit!!
      exit(1);
    } else {
      node->decimal.value = d_res;
      node->ty_id = TS_F64;
    }
  } else {
    node->decimal.value = f_res;
    node->ty_id = TS_F32;
  }

  return node;
}

ast_t* ast_mk_return(ast_t* argument) {
  // printf("ast_mk_return\n");
  ast_t* node = ast_new();
  node->type = AST_STMT_RETURN;

  node->ret.argument = argument;

  return node;
}

ast_t* ast_mk_break(ast_t* argument) {
  // printf("ast_mk_break\n");
  ast_t* node = ast_new();
  node->type = AST_STMT_BREAK;

  node->brk.argument = argument;

  return node;
}

ast_t* ast_mk_continue(ast_t* argument) {
  // printf("ast_mk_break\n");
  ast_t* node = ast_new();
  node->type = AST_STMT_CONTINUE;

  node->cont.argument = argument;

  return node;
}

ast_t* ast_mk_var_decl(ast_t* type, ast_t* id, ast_scope_t scope) {
  // printf("ast_mk_var_decl\n");
  ast_t* node = ast_new();
  node->type = AST_DTOR_VAR;

  node->var.id = id;
  node->var.type = type ? type : ast_mk_type(0, 0);
  node->var.scope = scope;

  return node;
}

ast_t* ast_mk_fn_decl(ast_t* id, ast_t* params, ast_t* ret_type, ast_t* body,
                      ast_t* attributes, int operator,
                      ast_function_type_t type) {
  // printf("ast_mk_fn_decl\n");
  ast_t* node = ast_new();
  node->type = AST_DECL_FUNCTION;

  node->func.ret_type = ret_type ? ret_type : ast_mk_type_auto();
  node->func.params = params = params ? params : ast_mk_list();
  node->func.operator= operator;
  node->func.type = type;

  // function id don't need to be resolved
  if (operator) {
    snprintf(ast_mk_buffer, 64, "operator_%d", operator);
    string* op_id = st_newc(ast_mk_buffer, st_enc_utf8);

    node->func.id = ast_mk_lit_id(op_id, false);
  } else {
    id->identifier.resolve = false;
    node->func.id = id;
  }

  if (body) {
    ast_mk_fn_decl_body(node, body);
  }

  if (attributes) {
    ast_mk_fn_decl_attributes(node, attributes);
  } else {
    node->func.attributes = ast_mk_list();
  }

  return node;
}

void ast_mk_fn_decl_body(ast_t* fn, ast_t* body) {
  // TODO maybe function scope
  body->block.scope = AST_SCOPE_BLOCK;
  fn->func.body = body;
}

void ast_mk_fn_decl_attributes(ast_t* fn, ast_t* attributes) {
  fn->func.attributes = attributes;

  ast_t* id = ast_get_attribute(attributes, st_newc("uid", st_enc_utf8));
  if (id) {
    fn->func.uid = id->attr.value->identifier.string;
  }

  // do not resolve!
  ast_t* attr_id;
  ast_t* attr_val;
  u64 i;
  for (i = 0; i < attributes->list.length; ++i) {
    attr_id = attributes->list.values[i]->attr.id;
    if (attr_id->type == AST_LIT_IDENTIFIER) {
      attr_id->identifier.resolve = false;
    }

    attr_val = attributes->list.values[i]->attr.value;
    if (attr_val && attr_val->type == AST_LIT_IDENTIFIER) {
      attr_val->identifier.resolve = false;
    }
  }
}

ast_t* ast_mk_fn_param(ast_t* id, ast_t* type, ast_t* def) {
  // printf("ast_mk_fn_param\n");
  ast_t* node = ast_new();
  node->type = AST_PARAMETER;

  id->identifier.resolve = false;
  node->param.id = id;
  node->param.type = type;
  node->param.def = def;

  // TODO assertions

  return node;
}

ast_t* ast_mk_binop(ast_t* left, int op, ast_t* right) {
  // printf("ast_mk_binop\n");
  ast_t* node = ast_new();
  node->type = AST_EXPR_BINOP;

  node->binop.left = left;
  node->binop.right = right;
  node->binop.operator= op;

  return node;
}

ast_t* ast_mk_assignament(ast_t* left, int op, ast_t* right) {
  // printf("ast_mk_assignament\n");
  ast_t* node = ast_new();
  node->type = AST_EXPR_ASSIGNAMENT;

  node->assignament.left = left;
  node->assignament.right = right;
  node->assignament.operator= op;

  return node;
}

ast_t* ast_mk_call_expr(ast_t* callee, ast_t* arguments) {
  // printf("ast_mk_call_expr\n");
  ast_t* node = ast_new();
  node->type = AST_EXPR_CALL;

  if (callee->type == AST_LIT_IDENTIFIER) {
    callee->identifier.resolve = false;
  }

  node->call.callee = callee;
  node->call.arguments = arguments;

  return node;
}

ast_t* ast_mk_type_auto() {
  return ast_mk_type(st_newc("auto", st_enc_utf8), 0);
}

ast_t* ast_mk_type_void() {
  return ast_mk_type(st_newc("void", st_enc_utf8), 0);
}

ast_t* ast_mk_type(string* id, ast_t* children) {
  fl_assert(children == 0 || children->type == AST_LIST);

  // printf("ast_mk_type\n");
  ast_t* node = ast_new();
  node->type = AST_TYPE;

  node->ty.id = id ? ast_mk_lit_id(id, false) : 0;
  node->ty.children = children;

  return node;
}

ast_t* ast_mk_type2(ast_t* id, ast_t* children) {
  fl_assert(children->type == AST_LIST);

  // printf("ast_mk_type\n");
  ast_t* node = ast_new();
  node->type = AST_TYPE;

  node->ty.id = id;
  node->ty.children = children;

  return node;
}

ast_t* ast_mk_comment(string* text) {
  // printf("ast_mk_comment\n");
  ast_t* node = ast_new();
  node->type = AST_STMT_COMMENT;

  node->comment.text = text;

  return node;
}

ast_t* ast_mk_lunary(ast_t* element, int operator) {
  // printf("ast_mk_lunary\n");
  ast_t* node = ast_new();
  node->type = AST_EXPR_LUNARY;

  node->lunary.element = element;
  node->lunary.operator= operator;

  return node;
}

ast_t* ast_mk_runary(ast_t* element, int operator) {
  // printf("ast_mk_runary\n");
  ast_t* node = ast_new();
  node->type = AST_EXPR_RUNARY;

  node->runary.element = element;
  node->runary.operator= operator;

  return node;
}

ast_t* ast_mk_if(ast_t* test, ast_t* block, ast_t* alternate) {
  // printf("ast_mk_if\n");
  ast_t* node = ast_new();
  node->type = AST_STMT_IF;

  node->if_stmt.test = test;
  node->if_stmt.block = block;
  node->if_stmt.alternate = alternate;

  return node;
}

ast_t* ast_mk_loop(ast_types_t type, ast_t* init, ast_t* pre_cond,
                   ast_t* update, ast_t* block, ast_t* post_cond) {
  // printf("ast_mk_loop\n");
  ast_t* node = ast_new();
  node->type = AST_STMT_LOOP;

  node->loop.init = init;
  node->loop.pre_cond = pre_cond;
  node->loop.update = update;
  node->loop.block = block;
  node->loop.post_cond = post_cond;
  node->loop.type = type;

  return node;
}

ast_t* ast_mk_struct_decl(ast_t* id, ast_t* tpls, ast_t* fields) {
  // printf("ast_mk_struct_decl\n");
  ast_t* node = ast_new();
  node->type = AST_DECL_STRUCT;

  id->identifier.resolve = false;
  node->structure.id = id;
  node->structure.fields = fields;
  node->structure.tpls = tpls;

  return node;
}

ast_t* ast_mk_struct_decl_field(ast_t* id, ast_t* type) {
  // printf("ast_mk_struct_decl_field\n");
  ast_t* node = ast_new();
  node->type = AST_DECL_STRUCT_FIELD;

  id->identifier.resolve = false;
  node->field.id = id;
  node->field.type = type;

  return node;
}

ast_t* ast_mk_struct_decl_alias(ast_t* name, ast_t* id) {
  // printf("ast_mk_struct_decl_field\n");
  ast_t* node = ast_new();
  node->type = AST_DECL_STRUCT_ALIAS;

  id->identifier.resolve = false;
  name->identifier.resolve = false;

  node->alias.name = name;
  node->alias.id = id;
  // node->field.index when type is created

  return node;
}

ast_t* ast_mk_member(ast_t* left, ast_t* property, bool expression,
                     bool brakets) {
  // printf("ast_mk_member\n");
  ast_t* node = ast_new();
  node->type = AST_EXPR_MEMBER;

  if (property->type == AST_LIT_IDENTIFIER && !expression) {
    property->identifier.resolve = false;
  }

  node->member.left = left;
  node->member.property = property;
  node->member.expression = expression;
  node->member.brakets = brakets;

  return node;
}

ast_t* ast_mk_sizeof(ast_t* type) {
  // printf("ast_mk_sizeof\n");
  fl_assert(type->type == AST_TYPE);

  ast_t* node = ast_new();
  node->type = AST_EXPR_SIZEOF;

  node->sof.type = type;

  return node;
}

ast_t* ast_mk_cast(ast_t* type, ast_t* element, bool unsafe) {
  // printf("ast_mk_cast\n");
  ast_t* node = ast_new();
  node->type = AST_CAST;

  node->cast.type = type;
  node->cast.element = element;
  node->cast.unsafe = unsafe;

  return node;
}

ast_t* ast_mk_import(ast_t* string_lit, bool forward) {
  // printf("ast_mk_import\n");
  ast_t* node = ast_new();
  node->type = AST_IMPORT;

  node->import.path = string_lit;
  node->import.forward = forward;

  return node;
}

ast_t* ast_mk_log(ast_t* list) {
  // printf("ast_mk_log\n");
  ast_t* node = ast_new();
  node->type = AST_STMT_LOG;
  node->log.print_expression = true; // TODO how to customize?

  node->log.list = list;

  return node;
}

ast_t* ast_mk_attribute(ast_t* id, ast_t* value) {
  // printf("ast_mk_log\n");
  ast_t* node = ast_new();
  node->type = AST_ATTRIBUTE;
  node->attr.id = id;
  node->attr.value = value;

  return node;
}

ast_t* ast_mk_template(ast_t* id, ast_t* block) {
  ast_t* node = ast_new();
  node->type = AST_DECL_TEMPLATE;
  node->tpl.id = id;
  id->identifier.resolve = false; // do not resolve it, it's a type

  return node;
}

ast_t* ast_mk_implement(ast_t* type_id, ast_t* type_list, ast_t* uid) {
  ast_t* node = ast_new();
  node->type = AST_IMPLEMENT;
  node->impl.type_id = type_id;
  node->impl.type_list = type_list;
  node->impl.uid = uid;
  uid->identifier.resolve = false; // do not resolve it, it's a type

  return node;
}

ast_t* ast_mk_new(ast_t* expr) {
  ast_t* node = ast_new();
  node->type = AST_NEW;
  node->new.expr = expr;

  return node;
}

ast_t* ast_mk_delete(ast_t* expr) {
  ast_t* node = ast_new();
  node->type = AST_DELETE;
  node->delete.expr = expr;

  return node;
}
