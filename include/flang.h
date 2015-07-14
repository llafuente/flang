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
#include <llvm-c/BitWriter.h>

//-
//- type declaration
//-

struct fl_struct_members;
typedef struct fl_struct_members fl_struct_members_t;

struct fl_enum_members;
typedef struct fl_enum_members fl_enum_members_t;

struct fl_type;
typedef struct fl_type fl_type_t;

struct fl_ast;
typedef struct fl_ast fl_ast_t;

struct fl_token_list;
typedef struct fl_token_list fl_token_list_t;

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
  FL_TK_FFI_C,
  FL_TK_RETURN,
  FL_TK_VAR,
  FL_TK_UNVAR,
  FL_TK_CONST,
  FL_TK_STATIC,
  FL_TK_GLOBAL,
  FL_TK_LPARENTHESIS,
  FL_TK_RPARENTHESIS,
  FL_TK_3DOT,
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
  FL_TK_COMMA,
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
  FL_TK_VOID,
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
  FL_TK_IF,
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

struct fl_parser_state {
  size_t current;
  fl_token_t* token;
  fl_token_t* prev_token;
  fl_token_t* next_token;
  size_t look_ahead_idx;
};

typedef struct fl_parser_state fl_psrstate_t;

enum fl_error_zone {
  FL_ERROR_SYNTAX = 1,
};

typedef enum fl_error_zone fl_error_zone_t;
enum fl_ast_type {
  FL_AST_PROGRAM = 1,
  FL_AST_BLOCK = 2,

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

  FL_AST_CAST = 27,

  // TODO FL_AST_DECL_VAR = 30
  FL_AST_DTOR_VAR = 31,

  FL_AST_TYPE = 40,

  FL_AST_DECL_FUNCTION = 50,
  FL_AST_PARAMETER = 51,
  FL_AST_STMT_RETURN = 52,

  FL_AST_STMT_IF = 60,

  FL_AST_STMT_COMMENT = 100,
  FL_AST_STMT_LOG = 101,

  FL_AST_ERROR = 255
};

typedef enum fl_ast_type fl_ast_type_t;

struct fl_ast {
  fl_token_t* token_start;
  fl_token_t* token_end;
  fl_ast_type_t type; // TODO enum
  void* codegen;      // space for codegen injection "userdata"
  struct fl_ast* parent;

  size_t real_ty_id;

  union {
    struct fl_ast_error {
      char* str;
      fl_error_zone_t zone;
    } err;

    struct fl_ast_program {
      // code as string
      string* code;
      // tokenizer result
      fl_token_list_t* tokens;
      // all types found in the code
      fl_type_t* types;
      size_t ntypes;
      // parsed core
      struct fl_ast* core;
      // actual code parsed
      struct fl_ast* body;
    } program;

    struct fl_ast_block {
      // list of statements
      struct fl_ast** body;
      // count
      size_t nbody;
    } block;

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
      size_t ty_id;
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
      struct fl_ast* right;
      fl_tokens_t operator;
    } lunary;

    struct fl_ast_expr_runary {
      struct fl_ast* element;
      fl_tokens_t operator;
    } runary;
    struct fl_ast_cast {
      fl_ast_t* to;
      fl_ast_t* right;
    } cast;
    struct fl_ast_dtor_variable {
      // TODO add type
      struct fl_ast* id;
      fl_ast_t* type;

      // codegen
      bool dirty;
      void* current;
    } var;
    struct fl_ast_idtype {
      size_t id; // id on fl_type_table
    } ty;
    struct fl_ast_decl_function {
      // TODO use fl_type_t*
      struct fl_ast* id;
      struct fl_ast** params;
      size_t nparams;
      struct fl_ast* body;
      struct fl_ast* ret_type;
      bool varargs;
      bool ffi; // TODO maybe ffi_type, 0 means flang, 1 means c...
    } func;
    struct fl_ast_parameter {
      struct fl_ast* id;
      struct fl_ast* type;
      struct fl_ast* def;
      struct fl_ast** assertions;
      size_t nassertions;
    } param;
    struct fl_ast_stmt_return {
      struct fl_ast* argument;
    } ret;
    struct fl_ast_stmt_if {
      struct fl_ast* test;
      struct fl_ast* block; // consequent
      struct fl_ast* alternate;
    } if_stmt;
    struct fl_ast_expr_call {
      fl_ast_t* callee;
      struct fl_ast** arguments;
      size_t narguments;
    } call;

    struct fl_ast_stmt_comment {
      string* text;
    } comment;
  };
};

// TODO resize support
struct fl_parser_stack {
  size_t current;
  fl_psrstate_t states[500];
};

typedef struct fl_parser_stack fl_psrstack_t;

enum fl_types {
  FL_VOID = 1,
  FL_NUMBER = 2,
  FL_POINTER = 3,
  FL_VECTOR = 4,
  FL_FUNCTION = 5,
  FL_STRUCT = 6,
  FL_ENUM = 7,

  FL_INFER = 10,

  FL_TEMPLATE0 = 20,
  FL_TEMPLATE1 = 21,
  FL_TEMPLATE2 = 22,
  FL_TEMPLATE3 = 23,
  FL_TEMPLATE4 = 24,
  FL_TEMPLATE5 = 25,
  FL_TEMPLATE6 = 26,
  FL_TEMPLATE7 = 27,
  FL_TEMPLATE8 = 28,
  FL_TEMPLATE9 = 29,
  // FL_REFERENCE = 12,
};

typedef enum fl_types fl_types_t;

// type must be unique
struct fl_type {
  fl_types_t of;
  void* codegen; // cache for codegen.

  // string, array, any are implemented inside the language
  // ref must be studied where should be, it's a very special pointer...
  union {
    // void is void :)

    // bool, iX, uX, fX
    struct fl_type_number {
      unsigned char bits;
      bool fp;
      bool sign;
    } number;

    struct fl_type_pointer {
      size_t to;
    } ptr;

    struct fl_type_vector {
      size_t size;
      size_t to;
    } vector;

    struct fl_type_function {
      string* name; // 0 means anonymous
      struct fl_type* ret;
      struct fl_type** params;
      size_t nparams;
    } fn;

    struct fl_type_struct {
      string* name;
      fl_struct_members_t** members;
    } agg; // aggregate

    struct fl_type_enum {
      string* name;
      fl_enum_members_t** members;
    } enu;
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
//- DEBUG MACROS
//-

#define cg_verbose(...)

#define cg_print(...)                                                          \
  do {                                                                         \
    fprintf(stderr, __VA_ARGS__);                                              \
  } while (false)
//#define cg_print(...) do{ } while ( false )

#define cg_error(...)                                                          \
  do {                                                                         \
    fprintf(stderr, __VA_ARGS__);                                              \
    fprintf(stderr, "@%s - %d\n", __FILE__, __LINE__);                         \
    exit(1);                                                                   \
  } while (false)

//-
//- MACROS
//-

#define FL_EXTERN extern

//- PARSER

#define PSR_READ_NAME(name) PSR_READ_##name

#define PSR_READ_DECL(name)                                                    \
  FL_EXTERN fl_ast_t* PSR_READ_NAME(name)(PSR_READ_HEADER)

#define PSR_READ_HEADER                                                        \
  fl_token_list_t* tokens, fl_psrstack_t* stack, fl_psrstate_t* state

#define PSR_READ_HEADER_SEND tokens, stack, state

#define PSR_READ_IMPL(name) fl_ast_t* PSR_READ_NAME(name)(PSR_READ_HEADER)

// , printf("%s\n", #name)
#define PSR_READ(name) PSR_READ_NAME(name)(tokens, stack, state)

#define PSR_AST_EXTEND(ast, name)                                              \
  PSR_READ_NAME(name)(tokens, stack, state, &ast)

// printf("%s %p type %d\n", __FUNCTION__, ast, ast->type);
// TODO handle errors when done :)
#define FL_TRY_READ(name)                                                      \
  fl_parser_look_ahead(stack, state);                                          \
  ast = PSR_READ(name);                                                        \
  if (ast) {                                                                   \
    /*handle errors*/                                                          \
    if (ast->type == FL_AST_ERROR) {                                           \
      fl_parser_rollback(stack, state);                                        \
      return ast;                                                              \
    }                                                                          \
    fl_parser_commit(stack, state);                                            \
    return ast;                                                                \
  }                                                                            \
  fl_parser_rollback(stack, state);

// calloc is necessary atm
#define PSR_AST_START(ast_type)                                                \
  fl_ast_t* ast = (fl_ast_t*)calloc(1, sizeof(fl_ast_t));                      \
  ast->token_start = state->token;                                             \
  ast->type = ast_type;

#define PSR_AST_DUMMY(name, ast_type)                                          \
  fl_ast_t* name = (fl_ast_t*)calloc(1, sizeof(fl_ast_t));                     \
  name->token_start = state->token;                                            \
  name->token_end = state->token;                                              \
  name->type = ast_type;

#define PSR_AST_END()                                                          \
  if (ast->type != FL_AST_ERROR) {                                             \
    ast->token_end = state->token;                                             \
  }

#define PSR_AST_RET()                                                          \
  PSR_AST_END();                                                               \
  return ast;

#define PSR_AST_RET_NULL()                                                     \
  if (ast) {                                                                   \
    fl_ast_delete(ast);                                                        \
    ast = 0;                                                                   \
  }                                                                            \
  return 0;

#define PSR_ACCEPT(string) fl_parser_accept(tokens, state, string)

#define PSR_ACCEPT_TOKEN(token_type)                                           \
  fl_parser_accept_token(tokens, state, token_type)

#define PSR_TEST_TOKEN(token_type) state->token->type == token_type

// , printf("next!\n")
#define PSR_NEXT() fl_parser_next(tokens, state)

/*
* new parser MACRO API
*/
#define PSR_START(target, ast_type) \
fl_ast_t* target = (fl_ast_t*)calloc(1, sizeof(fl_ast_t));                      \
target->token_start = state->token;                                             \
target->type = ast_type;

// read that can raise errors but 'dont throw'
#define PSR_SOFT_READ(target, name)                                            \
  fl_parser_look_ahead(stack, state);                                          \
  target = PSR_READ(name);                                                     \
  if (target) {                                                                \
    /*has errors?*/                                                            \
    if (target->type != FL_AST_ERROR) {                                        \
      fl_parser_commit(stack, state);                                          \
      return target;                                                           \
    }                                                                          \
    target = 0;                                                                \
  }                                                                            \
  fl_parser_rollback(stack, state);

#define PSR_RET_IF_ERROR(target, block)                                        \
  if (target->type == FL_AST_ERROR) {                                          \
    block return target;                                                       \
  }

#define PSR_RET_OK(target)                                                     \
  PSR_END(target);                                                             \
  return target;

#define PSR_END(target)                                                        \
  if (target->type != FL_AST_ERROR) {                                          \
    target->token_end = state->token;                                          \
  }

#define FL_CODEGEN_HEADER                                                      \
  fl_ast_t* node, LLVMBuilderRef builder, LLVMModuleRef module,                \
      LLVMContextRef context, LLVMBasicBlockRef current_block

#define FL_CODEGEN_HEADER_SEND node, builder, module, context, current_block
#define FL_CODEGEN_PASSTHROUGH builder, module, context, current_block

// target_ast allow to reuse current ast
#define PSR_SYNTAX_ERROR(target_ast, string)                                   \
  cg_print("(psr-err) %s\n", string);                                          \
  target_ast->type = FL_AST_ERROR;                                             \
  target_ast->token_end = state->token;                                        \
  target_ast->err.str = string;                                                \
  target_ast->err.zone = FL_ERROR_SYNTAX;

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

/* cldoc:begin-category(typesystem.c) */

FL_EXTERN bool ts_is_number(size_t id);
FL_EXTERN bool ts_is_fp(size_t id);
FL_EXTERN bool ts_is_int(size_t id);
FL_EXTERN size_t ts_get_bigger_typeid(size_t a, size_t b);

/* cldoc:end-category() */

/* cldoc:begin-category(parser.c) */

FL_EXTERN fl_ast_t* fl_parser(fl_token_list_t* tokens);

FL_EXTERN fl_ast_t* fl_parse(string* code);

FL_EXTERN fl_ast_t* fl_parse_utf8(char* str);

FL_EXTERN fl_ast_t* fl_parse_file(char* filename, bool core);

FL_EXTERN void fl_parse_core(fl_ast_t* ast);

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

FL_EXTERN void fl_parser_skipws(fl_token_list_t* tokens, fl_psrstate_t* state);

/* cldoc:end-category() */

/* cldoc:begin-category(parser-block.c) */

PSR_READ_DECL(block);
PSR_READ_DECL(program_block);
void PSR_READ_NAME(block_body)(PSR_READ_HEADER, fl_ast_t** extend);

/* cldoc:end-category() */

/* cldoc:begin-category(parser-literal.c) */

PSR_READ_DECL(literal);
PSR_READ_DECL(lit_null);
PSR_READ_DECL(lit_boolean);
PSR_READ_DECL(lit_string);
PSR_READ_DECL(lit_numeric);
PSR_READ_DECL(lit_identifier);

/* cldoc:end-category() */

/* cldoc:begin-category(parser-expression.c) */

PSR_READ_DECL(expression);
PSR_READ_DECL(expr_assignment);
PSR_READ_DECL(expr_assignment_full);
PSR_READ_DECL(expr_lhs);
PSR_READ_DECL(expr_primary);
PSR_READ_DECL(expr_conditional);
PSR_READ_DECL(expr_logical_or);
PSR_READ_DECL(expr_logical_and);
PSR_READ_DECL(expr_bitwise_or);
PSR_READ_DECL(expr_bitwise_xor);
PSR_READ_DECL(expr_bitwise_and);
PSR_READ_DECL(expr_equality);
PSR_READ_DECL(expr_relational);
PSR_READ_DECL(expr_shift);
PSR_READ_DECL(expr_additive);
PSR_READ_DECL(expr_multiplicative);
PSR_READ_DECL(expr_unary);
PSR_READ_DECL(expr_unary_left);
PSR_READ_DECL(expr_unary_right);
PSR_READ_DECL(expr_call);
PSR_READ_DECL(expr_argument);

typedef fl_ast_t* (*fl_read_cb_t)(PSR_READ_HEADER);
/* cldoc:end-category() */

/* cldoc:begin-category(parser-variable.c) */

PSR_READ_DECL(decl_variable);
PSR_READ_DECL(decl_variable_no_type);
PSR_READ_DECL(decl_variable_with_type);

/* cldoc:end-category() */

/* cldoc:begin-category(parser-type.c) */

PSR_READ_DECL(type);
PSR_READ_DECL(cast);
extern fl_type_t* fl_type_table;
extern size_t fl_type_size;
size_t fl_parser_get_typeid(fl_types_t wrapper, size_t child);
void fl_parser_init_types();

/* cldoc:end-category() */

/* cldoc:begin-category(parser-function.c) */

PSR_READ_DECL(decl_function);
PSR_READ_DECL(stmt_return);
PSR_READ_DECL(parameter_typed);
PSR_READ_DECL(parameter_notyped);
PSR_READ_DECL(parameter);

/* cldoc:end-category() */

/* cldoc:begin-category(parser-if.c) */

PSR_READ_DECL(stmt_if);

/* cldoc:end-category() */

/* cldoc:begin-category(parser-comment.c) */

PSR_READ_DECL(comment);

/* cldoc:end-category() */

/* cldoc:begin-category(ast.c) */

typedef bool (*fl_ast_cb_t)(fl_ast_t* node, fl_ast_t* parent, size_t level,
                            void* userdata);

FL_EXTERN void fl_ast_traverse(fl_ast_t* ast, fl_ast_cb_t cb, fl_ast_t* parent,
                               size_t level, void* userdata);

FL_EXTERN void fl_ast_reverse(fl_ast_t* ast, fl_ast_cb_t cb, fl_ast_t* parent,
                              size_t level, void* userdata);

FL_EXTERN void fl_ast_delete(fl_ast_t* ast);

FL_EXTERN void fl_ast_delete_list(fl_ast_t** list);

FL_EXTERN void fl_ast_parent(fl_ast_t* root);

FL_EXTERN fl_ast_t* fl_ast_search_decl_var(fl_ast_t* node, string* name);

FL_EXTERN size_t fl_ast_get_typeid(fl_ast_t* node);

FL_EXTERN bool fl_ast_is_pointer(fl_ast_t* node);

FL_EXTERN size_t fl_ast_ret_type(fl_ast_t* node);

/* cldoc:end-category() */

/* cldoc:begin-category(pass-inference.c) */

FL_EXTERN fl_ast_t* fl_pass_inference(fl_ast_t* node);

/* cldoc:end-category() */

/* cldoc:begin-category(codegen.c) */
FL_EXTERN void fl_interpreter(LLVMModuleRef module);
FL_EXTERN bool fl_to_bitcode(LLVMModuleRef module, const char* filename);
FL_EXTERN bool fl_to_ir(LLVMModuleRef module, const char* filename);

FL_EXTERN LLVMModuleRef fl_codegen(fl_ast_t* root, char* module_name);
FL_EXTERN LLVMValueRef fl_codegen_ast(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef fl_codegen_cast(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef fl_codegen_binop(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef fl_codegen_lit_number(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef fl_codegen_lit_boolean(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef fl_codegen_lit_string(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef fl_codegen_assignament(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef fl_codegen_dtor_var(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef fl_codegen_function(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef fl_codegen_return(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef fl_codegen_expr_call(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef fl_codegen_lunary(FL_CODEGEN_HEADER);
/* cldoc:end-category() */

/* cldoc:begin-category(codegen-type.c) */

FL_EXTERN LLVMTypeRef fl_codegen_get_type(fl_ast_t* node);
FL_EXTERN LLVMTypeRef fl_codegen_get_typeid(size_t id);
FL_EXTERN LLVMValueRef fl_codegen_cast_op(LLVMBuilderRef builder,
                                          size_t current, size_t expected,
                                          LLVMValueRef value);
/* cldoc:end-category() */

/* cldoc:begin-category(debug.c) */
FL_EXTERN void fl_print_type(size_t ty_id);
FL_EXTERN void fl_print_type_table();
FL_EXTERN bool fl_ast_debug_cb(fl_ast_t* node, fl_ast_t* parent, size_t level,
                               void* userdata);
FL_EXTERN void fl_ast_debug(fl_ast_t* node);
/* cldoc:end-category() */
