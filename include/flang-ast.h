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

enum ast_types {
  FL_AST_PROGRAM = 1,
  FL_AST_MODULE = 2,
  FL_AST_BLOCK = 3,
  FL_AST_LIST = 4, // TODO block->list, arguments->list, parameter->list etc.

  FL_AST_LIT_ARRAY = 10,
  FL_AST_LIT_OBJECT = 11,
  FL_AST_LIT_NUMERIC = 12,
  FL_AST_LIT_STRING = 13,
  FL_AST_LIT_BOOLEAN = 14,
  FL_AST_LIT_NULL = 15,
  FL_AST_LIT_IDENTIFIER = 16,

  FL_AST_EXPR = 20,
  FL_AST_EXPR_ASSIGNAMENT = 21,
  FL_AST_EXPR_CONDITIONAL = 22,
  FL_AST_EXPR_BINOP = 23,
  FL_AST_EXPR_LUNARY = 24,
  FL_AST_EXPR_RUNARY = 25,
  FL_AST_EXPR_CALL = 26,
  FL_AST_EXPR_MEMBER = 27,

  FL_AST_CAST = 28,

  // TODO FL_AST_DECL_VAR = 30
  FL_AST_DTOR_VAR = 31,

  FL_AST_TYPE = 40,
  FL_AST_DECL_STRUCT = 41,
  FL_AST_DECL_STRUCT_FIELD = 42,

  FL_AST_DECL_FUNCTION = 50,
  FL_AST_PARAMETER = 51,
  FL_AST_STMT_RETURN = 52,

  FL_AST_STMT_IF = 60,
  FL_AST_STMT_LOOP = 61,
  FL_AST_STMT_FOR = 62,     // not used
  FL_AST_STMT_WHILE = 63,   // not used
  FL_AST_STMT_DOWHILE = 64, // not used

  FL_AST_STMT_COMMENT = 100,
  FL_AST_STMT_LOG = 101,

  FL_AST_ERROR = 255
};

struct ast {
  tk_token_t* token_start;
  tk_token_t* token_end;
  ast_types_t type;
  ast_t* parent;

  // type id in ts_hashtable
  size_t ty_id;

  // codegen
  bool dirty;

  union {
    struct ast_error {
      char* str;
      fl_error_zone_t zone;
    } err;

    struct ast_program {
      string* path;
      // code as string
      string* code;
      // tokenizer result
      tk_token_list_t* tokens;
      // all types found in the code
      ty_t* types;
      size_t ntypes;
      // parsed core
      ast_t* core;
      // actual code parsed
      ast_t* body;
    } program;

    struct ast_block {
      // list of statements
      ast_t** body;
      // count
      size_t nbody;
    } block;

    struct ast_list {
      // list of statements
      ast_t** elements;
      // count
      size_t count;
    } list;

    struct ast_lit_boolean {
      bool value;
    } boolean;

    struct ast_lit_string {
      // single true, doubles false
      // TODO if support <<<XXX ... XXX; this should be changed to enum
      bool quoted;
      string* value;
    } string;

    struct ast_lit_numeric {
      double value;
    } numeric;

    struct ast_lit_identifier {
      string* string;
      // typesystem must search what this identifier means
      // variable, function, member
      bool resolve;

      ast_t* decl; // for codegen
    } identifier;

    struct ast_expr_assignament {
      ast_t* left;
      tk_tokens_t operator;
      ast_t* right;
    } assignament;

    struct ast_expr_binop {
      ast_t* left;
      tk_tokens_t operator;
      ast_t* right;
    } binop;

    struct ast_expr_lunary {
      ast_t* element;
      tk_tokens_t operator;
    } lunary;

    struct ast_expr_runary {
      ast_t* element;
      tk_tokens_t operator;
    } runary;

    struct ast_cast {
      ast_t* element;
      // use ty_id as type
    } cast;

    struct ast_dtor_variable {
      // TODO add type
      ast_t* id;
      ast_t* type;

      LLVMValueRef alloca;
    } var;

    // just for reference, but ty_id is used
    struct ast_idtype {
    } ty;

    struct ast_decl_struct {
      ast_t* id;
      ast_t* fields; // list
    } structure;     // aggregate

    struct ast_decl_struct_field {
      ast_t* type;
      ast_t* id;
    } field; // aggregate

    struct ast_decl_function {
      // TODO use ty_t*
      ast_t* id;
      string* uid; // realname in the module
      ast_t* params;
      size_t nparams;
      ast_t* body;
      ast_t* ret_type;
      bool varargs;
      bool ffi; // TODO maybe ffi_type, 0 means flang, 1 means c...
    } func;

    struct ast_parameter {
      ast_t* id;
      ast_t* type;
      ast_t* def;
      ast_t** assertions;
      size_t nassertions;

      LLVMValueRef alloca;
    } param;

    struct ast_stmt_return {
      ast_t* argument;
    } ret;

    struct ast_stmt_if {
      ast_t* test;
      ast_t* block; // consequent
      ast_t* alternate;
    } if_stmt;

    struct ast_stmt_loop {
      ast_t* init;
      ast_t* pre_cond;
      ast_t* update;
      ast_t* block;
      ast_t* post_cond;
      ast_types_t type;
    } loop;

    struct ast_expr_call {
      ast_t* callee;
      ast_t* decl; // set by typesystem
      ast_t* arguments;
      size_t narguments;
    } call;

    struct ast_expr_member {
      ast_t* left;
      ast_t* property;
      size_t idx;
      bool expression;
    } member;

    struct ast_stmt_comment {
      string* text;
    } comment;
  };
};
