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

enum ast_action { FL_AC_STOP = 0, FL_AC_CONTINUE, FL_AC_SKIP };

enum ast_cast_operations {
  FL_CAST_ERR = 0, // unkown
  FL_CAST_FPTOSI,  // LLVMBuildFPToSI
  FL_CAST_FPTOUI,  // LLVMBuildFPToUI
  FL_CAST_SITOFP,  // LLVMBuildSIToFP
  FL_CAST_UITOFP,  // LLVMBuildUIToFP
  FL_CAST_FPEXT,   // LLVMBuildFPExt
  FL_CAST_SEXT,    // LLVMBuildSExt
  FL_CAST_ZEXT,    // LLVMBuildZExt
  FL_CAST_FPTRUNC, // LLVMBuildFPTrunc
  FL_CAST_TRUNC,   // LLVMBuildTrunc
  FL_CAST_BITCAST, // LLVMBuildBitCast
  FL_CAST_AUTO,    // Function call
};

enum ast_scope {
  AST_SCOPE_GLOBAL = 1,
  AST_SCOPE_BLOCK,
  AST_SCOPE_FUNCTION,
  AST_SCOPE_TRANSPARENT
};
typedef enum ast_scope ast_scope_t;

enum ast_types {
  FL_AST_PROGRAM = 1,
  FL_AST_IMPORT = 2,
  FL_AST_MODULE = 3,
  FL_AST_BLOCK = 4,
  FL_AST_LIST = 5,

  FL_AST_LIT_ARRAY = 10,
  FL_AST_LIT_OBJECT = 11,
  FL_AST_LIT_INTEGER = 12,
  FL_AST_LIT_FLOAT = 13,
  FL_AST_LIT_STRING = 14,
  FL_AST_LIT_BOOLEAN = 15,
  FL_AST_LIT_NULL = 16,
  FL_AST_LIT_IDENTIFIER = 17,

  FL_AST_EXPR = 20,
  FL_AST_EXPR_ASSIGNAMENT = 21,
  FL_AST_EXPR_CONDITIONAL = 22,
  FL_AST_EXPR_BINOP = 23,
  FL_AST_EXPR_LUNARY = 24,
  FL_AST_EXPR_RUNARY = 25,
  FL_AST_EXPR_CALL = 26,
  FL_AST_EXPR_MEMBER = 27,
  FL_AST_EXPR_SIZEOF = 28,

  FL_AST_CAST = 29,

  // TODO FL_AST_DECL_VAR = 30
  FL_AST_DTOR_VAR = 31,

  FL_AST_TYPE = 40,
  FL_AST_DECL_STRUCT = 41,
  FL_AST_DECL_STRUCT_FIELD = 42,
  FL_AST_DECL_TEMPLATE = 43,

  FL_AST_DECL_FUNCTION = 50,
  FL_AST_PARAMETER = 51,
  FL_AST_STMT_RETURN = 52,
  FL_AST_STMT_BREAK = 53,
  FL_AST_STMT_CONTINUE = 54,

  FL_AST_STMT_IF = 60,
  FL_AST_STMT_LOOP = 61,
  FL_AST_STMT_FOR = 62,     // not used
  FL_AST_STMT_WHILE = 63,   // not used
  FL_AST_STMT_DOWHILE = 64, // not used

  FL_AST_STMT_COMMENT = 100,
  FL_AST_STMT_LOG = 101,
  FL_AST_ATTRIBUTE = 102,

  FL_AST_ERROR = 255
};

struct ast {
  int first_line;
  int first_column;
  int last_line;
  int last_column;

  ast_types_t type;
  ast_t* parent;

  size_t ty_id;

  // codegen
  bool dirty;

  union {
    struct ast_error {
      string* message;
      string* type;
    } err;

    struct ast_program {
      string* path;
      // code as string
      string* code;
      char* file;
      // all types found in the code
      ty_t* types;
      size_t ntypes;
      // actual code parsed
      ast_t* body;
    } program;

    struct ast_block {
      // list of statements
      ast_t* body;

      ast_t* parent_scope;

      ast_scope_t scope;

      hash_t* types;     // size_t with the type id
      hash_t* functions; // TODO array of decl?
      hash_t* variables; // ast_t* to declaration

      hash_t* uids; // bool, allocated by parser.y

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

    struct ast_lit_integer {
      // depending on ty_id signed should use one of these
      long int signed_value;
      long unsigned int unsigned_value;
    } integer;

    struct ast_lit_float {
      double value;
    } decimal;

    struct ast_lit_identifier {
      string* string;
      // typesystem must search what this identifier means
      // variable, function, member
      bool resolve;

      ast_t* decl; // for codegen
    } identifier;

    struct ast_expr_assignament {
      ast_t* left;
      int operator; // enum yytokentype
      ast_t* right;
    } assignament;

    struct ast_expr_binop {
      ast_t* left;
      int operator; // enum yytokentype
      ast_t* right;
    } binop;

    struct ast_expr_lunary {
      ast_t* element;
      int operator; // enum yytokentype
    } lunary;

    struct ast_expr_runary {
      ast_t* element;
      int operator; // enum yytokentype
    } runary;

    struct ast_cast {
      ast_cast_operations_t operation;
      ast_t* type;
      ast_t* element;
    } cast;

    struct ast_dtor_variable {
      // TODO add type
      ast_t* id;
      ast_t* type;
      ast_scope_t scope;

      LLVMValueRef alloca;
    } var;

    struct ast_idtype {
      ast_t* id;
      ast_t* child;
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
      string* uid; // final name
      ast_t* params;
      ast_t* body;
      ast_t* ret_type;
      ast_t* attributes;
      bool varargs;
      bool ffi; // TODO maybe ffi_type, 0 means flang, 1 means c...

      LLVMValueRef cg_decl;
    } func;

    struct ast_decl_template {
      ast_t* id;
    } tpl;

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

    struct ast_stmt_break {
      ast_t* argument;
    } brk;

    struct ast_stmt_continue {
      ast_t* argument;
    } cont;

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
      ast_t* arguments;
      size_t narguments;

      ast_t* decl; // set by typesystem
    } call;

    struct ast_expr_member {
      ast_t* left;
      ast_t* property;
      bool expression;

      size_t idx; // calc by typesystem
    } member;
    struct ast_expr_sizeof {
      ast_t* type;
    } sof;

    struct ast_stmt_comment {
      string* text;
    } comment;

    struct ast_import {
      // string literal
      bool forward;
      ast_t* path;

      bool imported;
    } import;

    struct ast_stmt_log {
      bool print_expression;
      ast_t* list;
    } log;

    struct ast_attribute {
      ast_t* id;
      ast_t* value;
    } attr;
  };
};
