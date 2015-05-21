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

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>
#include <stdint.h>

#include "stringc.h"

#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Transforms/Scalar.h>

//-
//- type declaration
//-

struct fl_struct_members;
typedef struct fl_struct_members fl_struct_members_t;

struct fl_enum_members;
typedef struct fl_enum_members fl_enum_members_t;

struct fl_type;
typedef struct fl_type fl_type_t;

//-
//- types
//-

struct fl_tk_state {
  size_t line;
  size_t column;
  char* itr;
  char* end;
};

typedef struct fl_tk_state fl_tk_state_t;

enum fl_tokens {
  FL_TK_UNKOWN,
  FL_TK_EOF,
  FL_TK_WHITESPACE,
  FL_TK_NEWLINE,
  FL_TK_LOG,
  FL_TK_FUNCTION,
  FL_TK_VAR,
  FL_TK_UNVAR,
  FL_TK_CONST,
  FL_TK_STATIC,
  FL_TK_GLOBAL,
  FL_TK_LPARANTHESIS,
  FL_TK_RPARANTHESIS,
  FL_TK_DOT,
  FL_TK_LBRACKET,
  FL_TK_RBRACKET,
  FL_TK_LCBRACKET,
  FL_TK_RCBRACKET,
  FL_TK_NEW,
  FL_TK_DELETE,
  FL_TK_RESIZE,
  FL_TK_TYPEOF,
  FL_TK_EXIST,
  FL_TK_COLON,
  FL_TK_SEMICOLON,
  FL_TK_EXCLAMATION,
  FL_TK_TILDE,
  FL_TK_SCOMMENT,
  FL_TK_MCOMMENT,
  FL_TK_ASTERISKEQUAL,
  FL_TK_ASTERISK,
  FL_TK_SLASHEQUAL,
  FL_TK_SLASH,
  FL_TK_MINUSQUAL,
  FL_TK_MINUS,
  FL_TK_MINUS2,
  FL_TK_PLUSEQUAL,
  FL_TK_PLUS,
  FL_TK_PLUS2,
  FL_TK_MODEQUAL,
  FL_TK_MOD,
  FL_TK_GT3EQUAL,
  FL_TK_GT2EQUAL,
  FL_TK_GT,
  FL_TK_GT2,
  FL_TK_GTE,
  FL_TK_LT,
  FL_TK_LT2EQUAL,
  FL_TK_LT2,
  FL_TK_LTE,
  FL_TK_EQUAL,
  FL_TK_QMARKEQUAL,
  FL_TK_EQUAL2,
  FL_TK_TEQUAL,
  FL_TK_EEQUAL,
  FL_TK_AT,
  FL_TK_ARRAY,
  FL_TK_QMARK,
  FL_TK_SQUOTE,
  FL_TK_DQUOTE,
  FL_TK_ANDEQUAL,
  FL_TK_AND,
  FL_TK_AND2,
  FL_TK_CEQUAL,
  FL_TK_CARET,
  FL_TK_OREQUAL,
  FL_TK_OR,
  FL_TK_OR2,
  FL_TK_BOOL,
  FL_TK_TRUE,
  FL_TK_FALSE,
  FL_TK_I8,
  FL_TK_I16,
  FL_TK_I32,
  FL_TK_I64,
  FL_TK_U8,
  FL_TK_U16,
  FL_TK_U32,
  FL_TK_U64,
  FL_TK_F32,
  FL_TK_F64,
  FL_TK_CAST,
  FL_TK_STRING,
  FL_TK_NULL,
  FL_TK_BACKTICK,
  FL_TK_DOLLAR,
  FL_TK_HASH
};

typedef enum fl_tokens fl_tokens_t;

struct fl_tokens_cfg {
  fl_tokens_t type;
  bool can_be_escaped;
  char* text;
  size_t text_s;
  char* close_text;
  size_t close_text_s;
  bool escapable;
};

typedef struct fl_tokens_cfg fl_tokens_cfg_t;

struct fl_token_pos {
  size_t line;
  size_t column;
};

typedef struct fl_token_pos fl_token_pos_t;

struct fl_token {
  string* string;
  fl_tokens_t type;
  fl_token_pos_t start;
  fl_token_pos_t end;
};

typedef struct fl_token fl_token_t;

struct fl_token_list {
  size_t size;
  fl_token_t tokens[];
};

enum fl_parser_result_level {
  FL_PR_ERROR_FINAL,
  FL_PR_WARNING,
  FL_PR_NOTICE,
  FL_PR_ERROR,
};

struct fl_parser_result {
  fl_token_t* token;
  char* text;
  enum fl_parser_result_level level;
};
typedef struct fl_parser_result fl_parser_result_t;

struct fl_parser_state {
  size_t current;
  fl_token_t* token;
  fl_token_t* prev_token;
  fl_token_t* next_token;
  size_t look_ahead_idx;
};

typedef struct fl_parser_state fl_psrstate_t;

enum fl_ast_type {
  FL_AST_PROGRAM = 1,

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

  // TODO FL_AST_DECL_VAR = 30
  FL_AST_DTOR_VAR = 31,

  FL_AST_TYPE = 40,

  FL_AST_STMT_LOG = 100,
};

typedef enum fl_ast_type fl_ast_type_t;

struct fl_ast {
  fl_token_t* token_start;
  fl_token_t* token_end;
  fl_ast_type_t type; // TODO enum
  void* codegen;      // space for codegen injection "userdata"

  union {
    struct fl_ast_program {
      struct fl_ast** body;
    } program;
    struct fl_ast_lit_boolean {
      bool value;
    } boolean;
    struct fl_ast_lit_string {
      // single true, doubles false
      // TODO if support <<<XXX ... XXX; this should be changed to enum
      bool quoted;
      string* value;
    } string;
    struct fl_ast_lit_numeric {
      double value;
    } numeric;
    struct fl_ast_lit_identifier {
      string* string;
    } identifier;
    struct fl_ast_expr_assignament {
      struct fl_ast* left;
      fl_tokens_t operator;
      struct fl_ast* right;
    } assignament;
    struct fl_ast_expr_binop {
      struct fl_ast* left;
      fl_tokens_t operator;
      struct fl_ast* right;
    } binop;
    struct fl_ast_expr_lunary {
      fl_tokens_t operator;
      struct fl_ast* element;
    } lunary;
    struct fl_ast_expr_runary {
      fl_tokens_t operator;
      struct fl_ast* element;
    } runary;
    struct fl_ast_dtor_variable {
      // TODO add type
      struct fl_ast* identifier;
      struct fl_ast* type;
    } var;
    struct fl_ast_idtype {
      // TODO use fl_type_t*
      fl_tokens_t of;
    } idtype;
  };
};

typedef struct fl_ast fl_ast_t;

typedef struct fl_token_list fl_token_list_t;

// TODO resize support
struct fl_parser_stack {
  size_t current;
  fl_psrstate_t states[500];
};

typedef struct fl_parser_stack fl_psrstack_t;

enum fl_types {
  FL_INT = 0,
  FL_FLOAT = 1,
  FL_BOOL = 2,
  FL_STRING = 3,
  FL_POINTER = 4,
  FL_FUNCTION = 5,
  FL_VOID = 6,
  FL_STRUCT = 7,
  FL_ARRAY = 8,
  FL_NULL = 9,
  FL_ANY = 10,
  FL_ENUM = 11,
  FL_REFERENCE = 12,
};

typedef enum fl_types fl_types_t;

struct fl_type {
  fl_types_t of;
  size_t size; // sizeof

  union {
    struct fl_type_int {
      unsigned char bits;
      bool sign;
    } in;

    struct fl_type_float {
      unsigned char size;
      bool sign;
    } fp;
    // bool has nothing
    // string has nothing
    struct fl_type_pointer {
      struct fl_type* to;
    } ptr;

    struct fl_type_function {
      string* name; // 0 means anonymous
      struct fl_type* ret;
      struct fl_type** args;
    } fn;
    // void has nothing

    struct fl_type_struct {
      string* name;
      fl_struct_members_t** members;
    } agg; // aggregate

    struct fl_type_array {
      struct fl_type* of;
    } arr;

    // null has nothing
    // any has nothing

    struct fl_type_enum {
      string* name;
      fl_enum_members_t** members;
    } enu;

    struct fl_type_ref {
      struct fl_type* to;
    } ref;
  };
};

struct fl_struct_members {
  string* name;
  fl_type_t* type;
};

struct fl_enum_members {
  string* name;
  size_t value;
};

//-
//- MACROS
//-

#define FL_EXTERN extern

#define FL_READER_FN(name) fl_read_##name

#define FL_READER_DECL(name)                                                   \
  FL_EXTERN fl_ast_t* FL_READER_FN(name)(FL_READER_HEADER);

#define FL_LIST_READER_DECL(name)                                              \
  FL_EXTERN fl_ast_t** FL_READER_FN(name)(FL_READER_HEADER);

#define FL_READER_HEADER                                                       \
  fl_token_list_t* tokens, fl_psrstack_t* stack, fl_psrstate_t* state

#define FL_READER_HEADER_SEND tokens, stack, state

#define FL_READER_IMPL(name) fl_ast_t* FL_READER_FN(name)(FL_READER_HEADER)

#define FL_LIST_READER_IMPL(name)                                              \
  fl_ast_t** FL_READER_FN(name)(FL_READER_HEADER)

// , printf("%s\n", #name)
#define FL_READ(name) FL_READER_FN(name)(tokens, stack, state)

// printf("%s %p type %d\n", __FUNCTION__, ast, ast->type);
// TODO handle errors when done :)
#define FL_TRY_READ(name)                                                      \
  fl_parser_look_ahead(stack, state);                                          \
  ast = FL_READ(name);                                                         \
  if (ast) {                                                                   \
    fl_parser_commit(stack, state);                                            \
    return ast;                                                                \
  }                                                                            \
  fl_parser_rollback(stack, state);

#define __FL_TRY_READ(name)                                                    \
  fl_parser_look_ahead(stack, state);                                          \
  ast = FL_READ(name);                                                         \
  if (ast) {                                                                   \
    fl_parser_commit(stack, state);                                            \
  } else {                                                                     \
    fl_parser_rollback(stack, state);                                          \
  }

// calloc is necessary atm
#define FL_AST_START(ast_type)                                                 \
  fl_ast_t* ast = (fl_ast_t*)calloc(1, sizeof(fl_ast_t));                      \
  ast->token_start = state->token;                                             \
  ast->type = ast_type;

#define FL_AST_END() ast->token_end = state->token

#define FL_RETURN_AST()                                                        \
  FL_AST_END();                                                                \
  return ast;

#define FL_RETURN_NOT_FOUND()                                                  \
  if (ast) {                                                                   \
    fl_ast_delete(ast);                                                        \
    ast = 0;                                                                   \
  }                                                                            \
  return 0;

#define FL_ACCEPT(string) fl_parser_accept(tokens, state, string)

#define FL_ACCEPT_TOKEN(token_type)                                            \
  fl_parser_accept_token(tokens, state, token_type)

// , printf("next!\n")
#define FL_NEXT() fl_parser_next(tokens, state)

#define FL_CODEGEN_HEADER                                                      \
  fl_ast_t* node, LLVMBuilderRef builder, LLVMModuleRef module,                \
      LLVMContextRef context

#define FL_CODEGEN_HEADER_SEND node, builder, module, context

//-
//- functions, global variables
//-

/* cldoc:begin-category(tokenizer.c) */

FL_EXTERN size_t fl_token_list_count;

FL_EXTERN fl_tokens_cfg_t fl_token_list[];

FL_EXTERN void fl_tokens_init();

FL_EXTERN void fl_tokens_delete(fl_token_list_t* tokens);

void fl_tokenize_cp_state(fl_tk_state_t* src, fl_tk_state_t* dst);

FL_EXTERN void fl_tokens_debug(fl_token_list_t* tokens);

FL_EXTERN fl_token_list_t* fl_tokenize(string* file);

/* cldoc:end-category() */

/* cldoc:begin-category(parser.c) */

FL_EXTERN fl_ast_t* fl_parser(fl_token_list_t* tokens);

FL_EXTERN fl_ast_t* fl_parse(string* str);
FL_EXTERN fl_ast_t* fl_parse_utf8(char* str);

FL_LIST_READER_DECL(body);

/* cldoc:end-category() */

/* cldoc:begin-category(parser-utils.c) */

FL_EXTERN bool fl_parser_next(fl_token_list_t* tokens, fl_psrstate_t* state);

FL_EXTERN bool fl_parser_prev(fl_token_list_t* tokens, fl_psrstate_t* state);

FL_EXTERN bool fl_parser_eof(fl_token_list_t* tokens, fl_psrstate_t* state);

FL_EXTERN bool fl_parser_accept(fl_token_list_t* tokens, fl_psrstate_t* state,
                                char* text);
FL_EXTERN bool fl_parser_accept_list(fl_token_list_t* tokens,
                                     fl_psrstate_t* state, char* text[],
                                     size_t text_count);

FL_EXTERN bool fl_parser_accept_token(fl_token_list_t* tokens,
                                      fl_psrstate_t* state,
                                      fl_tokens_t token_type);
FL_EXTERN bool fl_parser_accept_token_list(fl_token_list_t* tokens,
                                           fl_psrstate_t* state,
                                           fl_tokens_t token_type[],
                                           size_t tk_count);
/* cldoc:end-category() */

/* cldoc:begin-category(parser-stack.c) */

FL_EXTERN void fl_parser_stack_init(fl_psrstack_t* stack,
                                    fl_token_list_t* tokens,
                                    fl_psrstate_t* state);

FL_EXTERN void fl_parser_look_ahead(fl_psrstack_t* stack, fl_psrstate_t* state);

FL_EXTERN void fl_parser_commit(fl_psrstack_t* stack, fl_psrstate_t* state);

FL_EXTERN void fl_parser_rollback(fl_psrstack_t* stack, fl_psrstate_t* state);

FL_EXTERN fl_parser_result_t* fl_parser_expect(fl_token_list_t* tokens,
                                               fl_psrstate_t* state, char* text,
                                               char* err_msg, bool final);

FL_EXTERN void fl_parser_skipws(fl_token_list_t* tokens, fl_psrstate_t* state);

/* cldoc:end-category() */

/* cldoc:begin-category(parser-literal.c) */

FL_READER_DECL(literal);
FL_READER_DECL(lit_null);
FL_READER_DECL(lit_boolean);
FL_READER_DECL(lit_string);
FL_READER_DECL(lit_numeric);
FL_READER_DECL(lit_identifier);

/* cldoc:end-category() */

/* cldoc:begin-category(parser-expression.c) */

FL_READER_DECL(expression);
FL_READER_DECL(expr_assignment);
FL_READER_DECL(expr_assignment_full);
FL_READER_DECL(expr_lhs);
FL_READER_DECL(expr_conditional);
FL_READER_DECL(expr_logical_or);
FL_READER_DECL(expr_logical_and);
FL_READER_DECL(expr_bitwise_or);
FL_READER_DECL(expr_bitwise_xor);
FL_READER_DECL(expr_bitwise_and);
FL_READER_DECL(expr_equality);
FL_READER_DECL(expr_relational);
FL_READER_DECL(expr_shift);
FL_READER_DECL(expr_additive);
FL_READER_DECL(expr_multiplicative);
FL_READER_DECL(expr_unary);
FL_READER_DECL(expr_unary_left);
FL_READER_DECL(expr_unary_right);

typedef fl_ast_t* (*fl_reader_cb_t)(FL_READER_HEADER);
/* cldoc:end-category() */

/* cldoc:begin-category(parser-variable.c) */

FL_READER_DECL(decl_variable);
FL_READER_DECL(decl_variable_no_type);
FL_READER_DECL(decl_variable_with_type);

/* cldoc:end-category() */

/* cldoc:begin-category(parser-type.c) */

FL_READER_DECL(type);

/* cldoc:end-category() */

/* cldoc:begin-category(ast.c) */

typedef void (*fl_ast_cb_t)(fl_ast_t* node, fl_ast_t* parent, size_t level);

FL_EXTERN void fl_ast_traverse(fl_ast_t* ast, fl_ast_cb_t cb, fl_ast_t* parent,
                               size_t level);

FL_EXTERN void fl_ast_delete(fl_ast_t* ast);

FL_EXTERN void fl_ast_debug_cb(fl_ast_t* node, fl_ast_t* parent, size_t level);

/* cldoc:end-category() */

/* cldoc:begin-category(codegen.c) */

FL_EXTERN int fl_codegen(fl_ast_t* root, char* module_name);
FL_EXTERN LLVMValueRef fl_codegen_ast(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef fl_codegen_binop(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef fl_codegen_lit_number(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef fl_codegen_assignament(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef fl_codegen_dtor_var(FL_CODEGEN_HEADER);
/* cldoc:end-category() */
