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

#include "flang-parser.h"
#include "flang-debug.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>
#include <stdint.h>

#include "stringc.h"

#include <execinfo.h>

#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Transforms/Scalar.h>
#include <llvm-c/BitWriter.h>

#include "ext/uthash.h"

//-
//- type declaration
//-

struct fl_enum_members;
typedef struct fl_enum_members fl_enum_members_t;

struct fl_type;
typedef struct fl_type fl_type_t;

struct fl_type_cg;
typedef struct fl_type_cg fl_type_cg_t;

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
  FL_TK_UNKOWN = 0,
  FL_TK_EOF,
  FL_TK_WHITESPACE,
  FL_TK_NEWLINE,
  FL_TK_LOG,
  FL_TK_FUNCTION,
  FL_TK_FFI_C,
  FL_TK_RETURN,
  FL_TK_VAR,
  FL_TK_UNVAR,
  FL_TK_CONST = 10,
  FL_TK_STATIC,
  FL_TK_GLOBAL,
  FL_TK_LPARENTHESIS,
  FL_TK_RPARENTHESIS,
  FL_TK_3DOT,
  FL_TK_DOT,
  FL_TK_LBRACKET,
  FL_TK_RBRACKET,
  FL_TK_LCBRACKET,
  FL_TK_RCBRACKET = 20,
  FL_TK_EXIST,
  FL_TK_COLON,
  FL_TK_SEMICOLON,
  FL_TK_COMMA,
  FL_TK_EXCLAMATION,
  FL_TK_TILDE,
  FL_TK_SCOMMENT,
  FL_TK_MCOMMENT,
  FL_TK_ASTERISKEQUAL,
  FL_TK_ASTERISK = 30,
  FL_TK_SLASHEQUAL,
  FL_TK_SLASH,
  FL_TK_MINUSQUAL,
  FL_TK_MINUS,
  FL_TK_MINUS2,
  FL_TK_PLUSEQUAL,
  FL_TK_PLUS,
  FL_TK_PLUS2,
  FL_TK_MODEQUAL,
  FL_TK_MOD = 40,
  FL_TK_GT3EQUAL,
  FL_TK_GT2EQUAL,
  FL_TK_GT,
  FL_TK_GT2,
  FL_TK_GTE,
  FL_TK_LT,
  FL_TK_LT2EQUAL,
  FL_TK_LT2,
  FL_TK_LTE,
  FL_TK_EQUAL = 50,
  FL_TK_QMARKEQUAL,
  FL_TK_EQUAL2,
  FL_TK_TEQUAL,
  FL_TK_EEQUAL,
  FL_TK_AT,
  FL_TK_ARRAY,
  FL_TK_QMARK,
  FL_TK_SQUOTE,
  FL_TK_DQUOTE,
  FL_TK_ANDEQUAL = 60,
  FL_TK_AND,
  FL_TK_AND2,
  FL_TK_CEQUAL,
  FL_TK_CARET,
  FL_TK_OREQUAL,
  FL_TK_OR,
  FL_TK_OR2,
  // types
  FL_TK_VOID = 100,
  FL_TK_BOOL,
  FL_TK_TRUE,
  FL_TK_FALSE,
  FL_TK_I8,
  FL_TK_I16,
  FL_TK_I32,
  FL_TK_I64,
  FL_TK_U8,
  FL_TK_U16,
  FL_TK_U32 = 110,
  FL_TK_U64,
  FL_TK_F32,
  FL_TK_F64,
  FL_TK_NULL,
  FL_TK_STRUCT,

  FL_TK_IF = 120,
  FL_TK_ELSE,
  FL_TK_FOR,
  FL_TK_WHILE,
  FL_TK_DO,
  FL_TK_NEW,
  FL_TK_DELETE,
  FL_TK_RESIZE,
  FL_TK_TYPEOF,
  FL_TK_CAST,
  FL_TK_STRING,
  FL_TK_BACKTICK,
  FL_TK_DOLLAR,
  FL_TK_HASH
};

typedef enum fl_tokens fl_tokens_t;

struct fl_tokens_cfg {
  fl_tokens_t type;
  char* text;
  size_t text_s;
  char* close_text;
  size_t close_text_s;
  bool escapable;
  bool is_punctuation;
  bool is_reserved_word;
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
  FL_AST_LIST = 3, // TODO block->list, arguments->list, parameter->list etc.

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

typedef enum fl_ast_type fl_ast_type_t;

struct fl_ast {
  fl_token_t* token_start;
  fl_token_t* token_end;
  fl_ast_type_t type; // TODO enum
  struct fl_ast* parent;

  size_t ty_id;

  // codegen
  bool dirty;

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

    struct fl_ast_list {
      // list of statements
      struct fl_ast** elements;
      // count
      size_t count;
    } list;

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
      // typesystem must search what this identifier means
      // variable, function, member
      bool resolve;
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
      struct fl_ast* element;
      fl_tokens_t operator;
    } lunary;

    struct fl_ast_expr_runary {
      struct fl_ast* element;
      fl_tokens_t operator;
    } runary;

    struct fl_ast_cast {
      fl_ast_t* element;
      // use ty_id as type
    } cast;

    struct fl_ast_dtor_variable {
      // TODO add type
      struct fl_ast* id;
      fl_ast_t* type;

      LLVMValueRef alloca;
    } var;

    // just for reference, but ty_id is used
    struct fl_ast_idtype {
    } ty;

    struct fl_ast_decl_struct {
      fl_ast_t* id;
      fl_ast_t* fields; // list
    } structure;        // aggregate

    struct fl_ast_decl_struct_field {
      fl_ast_t* type;
      fl_ast_t* id;
    } field; // aggregate

    struct fl_ast_decl_function {
      // TODO use fl_type_t*
      struct fl_ast* id;
      struct fl_ast* params;
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

      LLVMValueRef alloca;
    } param;

    struct fl_ast_stmt_return {
      struct fl_ast* argument;
    } ret;

    struct fl_ast_stmt_if {
      struct fl_ast* test;
      struct fl_ast* block; // consequent
      struct fl_ast* alternate;
    } if_stmt;

    struct fl_ast_stmt_loop {
      struct fl_ast* init;
      struct fl_ast* pre_cond;
      struct fl_ast* update;
      struct fl_ast* block;
      struct fl_ast* post_cond;
      fl_ast_type_t type;
    } loop;

    struct fl_ast_expr_call {
      fl_ast_t* callee;
      struct fl_ast* arguments;
      size_t narguments;
    } call;

    struct fl_ast_expr_member {
      fl_ast_t* left;
      fl_ast_t* property;
    } member;

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
  FL_POINTER = 3, // wrapper
  FL_VECTOR = 4,  // wrapper
  FL_FUNCTION = 5,
  FL_STRUCT = 6,
  FL_ENUM = 7, // TODO this is in fact an "int"

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
  string* id;

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
      size_t size; // TODO 0 resizable?
      size_t to;
    } vector;

    struct fl_type_function {
      size_t ret;
      size_t* params;
      size_t nparams;
      bool varargs;

      fl_ast_t* decl;
    } func;

    struct fl_type_struct {
      size_t* fields;
      size_t nfields;

      fl_ast_t* decl;
    } structure;

    struct fl_type_enum {
      size_t* members;
      size_t nmembers;

      fl_ast_t* decl;
    } enu;
  };
};

struct fl_type_cg {
  size_t id;
  fl_ast_t* decl;

  UT_hash_handle hh; // makes this structure hashable
  char name[64];     // key (string is WITHIN the structure)
};

struct fl_enum_members {
  string* name;
  size_t value;
};

//-
//- MACROS
//-

#define FL_EXTERN extern

#define FL_CODEGEN_HEADER                                                      \
  fl_ast_t* node, LLVMBuilderRef builder, LLVMModuleRef module,                \
      LLVMContextRef context, LLVMBasicBlockRef* current_block,                \
      LLVMValueRef parent

#define FL_CODEGEN_HEADER_SEND                                                 \
  node, builder, module, context, current_block, parent
#define FL_CODEGEN_PASSTHROUGH builder, module, context, current_block, parent

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

extern fl_type_t* fl_type_table;
extern size_t fl_type_size;
extern fl_type_cg_t* ts_hashtable;

FL_EXTERN bool ts_is_number(size_t id);
FL_EXTERN bool ts_is_fp(size_t id);
FL_EXTERN bool ts_is_int(size_t id);
FL_EXTERN size_t ts_get_bigger_typeid(size_t a, size_t b);
FL_EXTERN fl_ast_t* ts_pass(fl_ast_t* node);

// return the unique typeid given ret + arguments
FL_EXTERN size_t ts_fn_create(fl_ast_t* decl);
// return the unique typeid given fields
FL_EXTERN size_t ts_struct_create(fl_ast_t* decl);

FL_EXTERN size_t ts_named_typeid(string* id);
FL_EXTERN fl_type_cg_t* ts_named_type(string* id);
FL_EXTERN size_t ts_struct_property_type(size_t id, string* property);
FL_EXTERN size_t ts_struct_property_idx(size_t id, string* property);

FL_EXTERN size_t ts_fn_typeid(fl_ast_t* id);
FL_EXTERN size_t ts_var_typeid(fl_ast_t* id);

FL_EXTERN void ts_init();
FL_EXTERN void ts_exit();

/* cldoc:end-category() */

typedef fl_ast_t* (*psr_read_t)(PSR_READ_HEADER);

/* cldoc:begin-category(parser.c) */

FL_EXTERN fl_ast_t* fl_parser(fl_token_list_t* tokens, bool core);

FL_EXTERN fl_ast_t* fl_parse(string* code, bool core);

FL_EXTERN fl_ast_t* fl_parse_utf8(char* str);

FL_EXTERN fl_ast_t* fl_parse_file(char* filename, bool core);

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
FL_EXTERN fl_ast_t* psr_read_list(psr_read_t* arr, size_t length,
                                  PSR_READ_HEADER);
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
PSR_READ_DECL(lit_identifier_rw);

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
/* cldoc:end-category() */

/* cldoc:begin-category(parser-variable.c) */

PSR_READ_DECL(decl_variable);
PSR_READ_DECL(decl_variable_no_type);
PSR_READ_DECL(decl_variable_with_type);

/* cldoc:end-category() */

/* cldoc:begin-category(parser-type.c) */

PSR_READ_DECL(type);
PSR_READ_DECL(decl_struct);
PSR_READ_DECL(cast);

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

/* cldoc:begin-category(parser-loops.c) */

PSR_READ_IMPL(stmt_for);
PSR_READ_IMPL(stmt_while);
PSR_READ_IMPL(stmt_dowhile);

/* cldoc:end-category() */

/* cldoc:begin-category(parser-comment.c) */

PSR_READ_DECL(comment);

/* cldoc:end-category() */

/* cldoc:begin-category(ast.c) */

typedef bool (*fl_ast_cb_t)(fl_ast_t* node, fl_ast_t* parent, size_t level,
                            void* userdata);

typedef bool (*fl_ast_ret_cb_t)(fl_ast_t* node, fl_ast_t* parent, size_t level,
                                void* userdata, void** ret);

FL_EXTERN void fl_ast_traverse(fl_ast_t* ast, fl_ast_cb_t cb, fl_ast_t* parent,
                               size_t level, void* userdata);

FL_EXTERN void* fl_ast_reverse(fl_ast_t* ast, fl_ast_ret_cb_t cb,
                               fl_ast_t* parent, size_t level, void* userdata);

FL_EXTERN void fl_ast_delete(fl_ast_t* ast);

FL_EXTERN void fl_ast_delete_props(fl_ast_t* ast);

FL_EXTERN void fl_ast_delete_list(fl_ast_t** list);

FL_EXTERN void fl_ast_parent(fl_ast_t* root);

FL_EXTERN fl_ast_t* fl_ast_search_decl_var(fl_ast_t* node, string* name);

FL_EXTERN size_t fl_ast_get_typeid(fl_ast_t* node);

FL_EXTERN bool fl_ast_is_pointer(fl_ast_t* node);

FL_EXTERN size_t fl_ast_ret_type(fl_ast_t* node);

FL_EXTERN fl_ast_t* fl_ast_find_fn_decl(fl_ast_t* identifier);

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
FL_EXTERN LLVMValueRef fl_codegen_if(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef fl_codegen_loop(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef fl_codegen_left_identifier(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef fl_codegen_right_identifier(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef fl_codegen_lhs(FL_CODEGEN_HEADER);
/* cldoc:end-category() */

/* cldoc:begin-category(codegen-type.c) */

FL_EXTERN LLVMTypeRef
fl_codegen_get_type(fl_ast_t* node, LLVMContextRef context);
FL_EXTERN LLVMTypeRef fl_codegen_get_typeid(size_t id, LLVMContextRef context);
FL_EXTERN LLVMValueRef
fl_codegen_cast_op(LLVMBuilderRef builder, size_t current, size_t expected,
                   LLVMValueRef value, LLVMContextRef context);
/* cldoc:end-category() */

/* cldoc:begin-category(debug.c) */
FL_EXTERN void fl_print_type(size_t ty_id);
FL_EXTERN void fl_print_type_table();
FL_EXTERN bool fl_ast_debug_cb(fl_ast_t* node, fl_ast_t* parent, size_t level,
                               void* userdata);
FL_EXTERN void fl_ast_debug(fl_ast_t* node);
/* cldoc:end-category() */
