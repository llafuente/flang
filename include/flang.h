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

#include <execinfo.h>

#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Transforms/Scalar.h>
#include <llvm-c/BitWriter.h>

#include "ext/uthash.h"
#include "ext/array.h"

//-
//- type pre-declaration
//-

enum fl_error_zone { FL_ERROR_SYNTAX = 1 };
typedef enum fl_error_zone fl_error_zone_t;

enum ast_types;
typedef enum ast_types ast_types_t;

struct ast;
typedef struct ast ast_t;

struct ts_type;
typedef struct ts_type ty_t;

enum ts_types;
typedef enum ts_types ts_types_t;

struct tk_position;
typedef struct tk_position tk_position_t;

enum tk_tokens;
typedef enum tk_tokens tk_tokens_t;

struct tk_token;
typedef struct tk_token tk_token_t;

struct tk_token_list;
typedef struct tk_token_list tk_token_list_t;

struct tk_state;
typedef struct tk_state tk_state_t;

struct tk_token_cfg;
typedef struct tk_token_cfg tk_token_cfg_t;

struct psr_state;
typedef struct psr_state fl_psrstate_t;

struct psr_stack;
typedef struct psr_stack fl_psrstack_t;

struct ts_typeh;
typedef struct ts_typeh ts_typeh_t;

#include "flang-typesystem.h"
#include "flang-tokenizer.h"
#include "flang-parser.h"
#include "flang-debug.h"
#include "flang-ast.h"

//-
//- MACROS
//-

#define FL_EXTERN extern

#define FL_CODEGEN_HEADER                                                      \
  ast_t* node, LLVMBuilderRef builder, LLVMModuleRef module,                   \
      LLVMContextRef context, LLVMBasicBlockRef* current_block,                \
      LLVMValueRef parent

#define FL_CODEGEN_HEADER_SEND                                                 \
  node, builder, module, context, current_block, parent
#define FL_CODEGEN_PASSTHROUGH builder, module, context, current_block, parent

//-
//- functions, global variables
//-

/* cldoc:begin-category(tokenizer.c) */

FL_EXTERN size_t tk_token_list_s;

FL_EXTERN tk_token_cfg_t tk_token_list[];

FL_EXTERN void tk_tokens_delete(tk_token_list_t* tokens);

void tk_cp_state(tk_state_t* src, tk_state_t* dst);

FL_EXTERN void tk_dump(tk_token_list_t* tokens);

FL_EXTERN tk_token_list_t* fl_tokenize(string* file);

/* cldoc:end-category() */

/* cldoc:begin-category(typesystem/typesystem.c) */

extern ty_t* ts_type_table;
extern size_t ts_type_size_s;
extern ts_typeh_t* ts_hashtable;

FL_EXTERN bool ts_is_pointer(size_t id);
FL_EXTERN bool ts_is_struct(size_t id);
FL_EXTERN bool ts_is_number(size_t id);
FL_EXTERN bool ts_is_fp(size_t id);
FL_EXTERN bool ts_is_int(size_t id);
FL_EXTERN size_t ts_wapper_typeid(ts_types_t wrapper, size_t child);
FL_EXTERN size_t ts_get_bigger_typeid(size_t a, size_t b);
FL_EXTERN ast_t* ts_pass(ast_t* node);

// return the unique typeid given ret + arguments
FL_EXTERN size_t ts_fn_create(ast_t* decl);
// return the unique typeid given fields
FL_EXTERN size_t ts_struct_create(ast_t* decl);
FL_EXTERN size_t ts_struct_idx(ast_t* decl, string* id);

FL_EXTERN size_t ts_named_typeid(string* id);
FL_EXTERN ts_typeh_t* ts_named_type(string* id);
FL_EXTERN size_t ts_struct_property_type(size_t id, string* property);
FL_EXTERN size_t ts_struct_property_idx(size_t id, string* property);

FL_EXTERN size_t ts_fn_typeid(ast_t* id);
FL_EXTERN ast_t* ts_find_fn_decl(string* id, ast_t* args);
FL_EXTERN size_t ts_var_typeid(ast_t* id);

FL_EXTERN void ts_init();
FL_EXTERN void ts_exit();

/* cldoc:end-category() */

/* cldoc:begin-category(typesystem/cast.c) */
FL_EXTERN bool ts_castable(size_t aty_id, size_t bty_id);
/* cldoc:end-category() */

typedef ast_t* (*psr_read_t)(PSR_READ_HEADER);

/* cldoc:begin-category(parser.c) */

FL_EXTERN ast_t* fl_parser(tk_token_list_t* tokens, bool core);

FL_EXTERN ast_t* fl_parse(string* code, bool core);

FL_EXTERN ast_t* fl_parse_utf8(char* str);

FL_EXTERN ast_t* fl_parse_file(char* filename, bool core);

/* cldoc:end-category() */

/* cldoc:begin-category(parser-utils.c) */

FL_EXTERN bool psr_next(tk_token_list_t* tokens, fl_psrstate_t* state);

FL_EXTERN bool psr_prev(tk_token_list_t* tokens, fl_psrstate_t* state);

FL_EXTERN bool psr_eof(tk_token_list_t* tokens, fl_psrstate_t* state);

FL_EXTERN bool psr_accept(tk_token_list_t* tokens, fl_psrstate_t* state,
                          char* text);
FL_EXTERN bool psr_accept_list(tk_token_list_t* tokens, fl_psrstate_t* state,
                               char* text[], size_t text_count);

FL_EXTERN bool psr_accept_token(tk_token_list_t* tokens, fl_psrstate_t* state,
                                tk_tokens_t token_type);
FL_EXTERN bool psr_accept_token_list(tk_token_list_t* tokens,
                                     fl_psrstate_t* state,
                                     tk_tokens_t token_type[], size_t tk_count);
FL_EXTERN ast_t* psr_read_list(psr_read_t* arr, size_t length, PSR_READ_HEADER);
/* cldoc:end-category() */

/* cldoc:begin-category(parser-stack.c) */

FL_EXTERN void psr_stack_init(fl_psrstack_t* stack, tk_token_list_t* tokens,
                              fl_psrstate_t* state);

FL_EXTERN void psr_look_ahead(fl_psrstack_t* stack, fl_psrstate_t* state);

FL_EXTERN void psr_commit(fl_psrstack_t* stack, fl_psrstate_t* state);

FL_EXTERN void psr_rollback(fl_psrstack_t* stack, fl_psrstate_t* state);

FL_EXTERN void psr_skipws(tk_token_list_t* tokens, fl_psrstate_t* state);

/* cldoc:end-category() */

/* cldoc:begin-category(parser-block.c) */

PSR_READ_DECL(block);
PSR_READ_DECL(program_block);
void PSR_READ_NAME(block_body)(PSR_READ_HEADER, ast_t** extend);

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

/* cldoc:begin-category(parser-load.c) */

PSR_READ_IMPL(pp_load);

/* cldoc:end-category() */

/* cldoc:begin-category(parser-comment.c) */

PSR_READ_DECL(comment);

/* cldoc:end-category() */

/* cldoc:begin-category(ast/ast.c) */

/**
 * @returns if the iteration must stop
 */
typedef bool (*ast_cb_t)(ast_t* node, ast_t* parent, size_t level,
                         void* userdata_in, void* userdata_out);

FL_EXTERN void ast_parent(ast_t* root);

FL_EXTERN ast_t* ast_search_decl_var(ast_t* node, string* name);

FL_EXTERN size_t ast_get_typeid(ast_t* node);

FL_EXTERN bool ast_is_pointer(ast_t* node);

FL_EXTERN size_t ast_ret_type(ast_t* node);

FL_EXTERN ast_t* ast_find_fn_decl(ast_t* identifier);

FL_EXTERN array* ast_find_fn_decls(ast_t* node, string* id);

/* cldoc:end-category() */

/* cldoc:begin-category(ast/traverse.c) */
FL_EXTERN void ast_traverse(ast_t* ast, ast_cb_t cb, ast_t* parent,
                            size_t level, void* userdata_in,
                            void* userdata_out);
/* cldoc:end-category() */

/* cldoc:begin-category(ast/reverse.c) */
FL_EXTERN void ast_reverse(ast_t* ast, ast_cb_t cb, ast_t* parent, size_t level,
                           void* userdata_in, void* userdata_out);
/* cldoc:end-category() */

/* cldoc:begin-category(ast/alloc.c) */

FL_EXTERN ast_t* ast_new();

FL_EXTERN void ast_delete(ast_t* ast);

FL_EXTERN void ast_delete_props(ast_t* ast);

FL_EXTERN void ast_delete_list(ast_t** list);

/* cldoc:end-category() */

/* cldoc:begin-category(ast/ast-static.c) */
FL_EXTERN bool ast_is_static(ast_t* node);
/* cldoc:end-category() */

/* cldoc:begin-category(pass-inference.c) */

FL_EXTERN ast_t* ts_pass_inference(ast_t* node);

/* cldoc:end-category() */

/* cldoc:begin-category(codegen.c) */
FL_EXTERN void fl_interpreter(LLVMModuleRef module);
FL_EXTERN bool fl_to_bitcode(LLVMModuleRef module, const char* filename);
FL_EXTERN bool fl_to_ir(LLVMModuleRef module, const char* filename);

FL_EXTERN LLVMModuleRef fl_codegen(ast_t* root, char* module_name);
FL_EXTERN LLVMValueRef cg_ast(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef cg_cast(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef cg_cast(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef cg_binop(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef cg_lit_number(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef cg_lit_boolean(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef cg_lit_string(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef cg_assignament(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef cg_dtor_var(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef cg_function(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef cg_return(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef cg_expr_call(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef cg_lunary(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef cg_if(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef cg_loop(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef cg_left_identifier(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef cg_right_identifier(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef cg_right_member(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef cg_left_member(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef cg_lhs(FL_CODEGEN_HEADER);
/* cldoc:end-category() */

/* cldoc:begin-category(codegen-type.c) */
FL_EXTERN LLVMTypeRef cg_get_type(ast_t* node, LLVMContextRef context);
FL_EXTERN LLVMTypeRef cg_get_typeid(size_t id, LLVMContextRef context);
FL_EXTERN LLVMValueRef cg_cast_op(LLVMBuilderRef builder, size_t current,
                                  size_t expected, LLVMValueRef value,
                                  LLVMContextRef context);
/* cldoc:end-category() */

/* cldoc:begin-category(debug.c) */
// 4 by default
extern int log_debug_level;
/* cldoc:end-category() */

FL_EXTERN void ty_dump(size_t ty_id);
FL_EXTERN void ty_dump_table();
FL_EXTERN void ast_dump(ast_t* node);
FL_EXTERN void ast_dump_one(ast_t* node);
