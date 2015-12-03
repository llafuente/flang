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

/*
#undef NDEBUG
#include <assert.h>
*/

#ifndef ASAN
#error : ASAN must be either 0 or 1, use -DASAN=X
#endif

extern void __sanitizer_print_stack_trace();

#if ASAN == 1
#define assert(check)                                                          \
  if (!(check)) {                                                              \
    __sanitizer_print_stack_trace();                                           \
  }
//    fprintf("assert: %s\n", #check);

#else
#undef NDEBUG
#include <assert.h>
#endif

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
#include "ext/pool.h"

//-
//- type pre-declaration
//-

enum ast_types;
typedef enum ast_types ast_types_t;

enum ast_cast_operations;
typedef enum ast_cast_operations ast_cast_operations_t;

enum ast_action;
typedef enum ast_action ast_action_t;
struct ast;
typedef struct ast ast_t;

struct ts_type;
typedef struct ts_type ty_t;

enum ts_types;
typedef enum ts_types ts_types_t;

struct psr_state;
typedef struct psr_state fl_psrstate_t;

struct psr_stack;
typedef struct psr_stack fl_psrstack_t;

struct ts_type_hash;
typedef struct ts_type_hash ts_type_hash_t;

#include "grammar/parser.h"
//#include "grammar/tokens.h"
#include "flang-typesystem.h"
#include "flang-debug.h"
#include "flang-ast.h"

//-
//- MACROS
//-

#define FL_EXTERN extern

#define FL_CODEGEN_HEADER                                                      \
  ast_t* node, LLVMBuilderRef builder, LLVMModuleRef module,                   \
      LLVMContextRef context, LLVMBasicBlockRef* current_block,                \
      LLVMValueRef* parent

#define FL_CODEGEN_HEADER_SEND                                                 \
  node, builder, module, context, current_block, parent
#define FL_CODEGEN_PASSTHROUGH builder, module, context, current_block, parent

//-
//- functions, global variables
//-

/* cldoc:begin-category(flang.c) */

extern array* identifiers;

/* Initiaze global variables and memory pool.
 * Must be called before anything else.
 *
 * see: [flang_exit](#flang_exit)
 *
 * @input str input string
 * @return root ast node
 */
FL_EXTERN void flang_init();

/* Free global variables and the memory pool
 *
 * see: [flang_init](#flang_init)
 *
 * @input str input string
 * @return root ast node
 */
FL_EXTERN void flang_exit(ast_t* root);

/* cldoc:end-category() */

/* cldoc:begin-category(parser/parser-utils.c) */

/* Parse an utf8 string and return the root ast node.
 *
 * see: [fl_parse_file](#fl_parse_file)
 *
 * @input str input string
 * @return root ast node
 */
FL_EXTERN ast_t* fl_parse_utf8(char* str);

/* Parse an utf8 string and return the root ast node
 * with import core at the beginning. So you can later call
 * ast_load_imports and get the full ast.
 *
 * see: [fl_parse_main_file](#fl_parse_main_file)
 *
 * @input str input string
 * @return root ast node
 */
FL_EXTERN ast_t* fl_parse_main_utf8(char* str);

/* Return file contents parsed.
 *
 * see: [fl_parse_utf8](#fl_parse_utf8)
 *
 * @input filename path to file
 * @return root ast node
 */
FL_EXTERN ast_t* fl_parse_file(const char* filename);

/* Return file contents parsed with import core at the beginning.
 * So you can later call ast_load_imports and get the full ast.
 *
 * see: [fl_parse_main_utf8](#fl_parse_main_utf8)
 *
 * @input filename path to file
 * @return root ast node
 */
FL_EXTERN ast_t* fl_parse_main_file(const char* filename);

/* Help function, exported to easy test code.
 *
 * @input filename path to file
 * @return file contents as string
 */
FL_EXTERN string* fl_file_to_string(const char* filename);

/* cldoc:end-category() */

/* cldoc:begin-category(typesystem/type-dump.c) */

/* Get string representation of given type.
 * Do not need to free ther string, it's allocated in the pool.
 *
 * @ty_id type id
 * @return type as string
 */
FL_EXTERN string* ty_to_string(size_t ty_id);
FL_EXTERN void ty_dump(size_t ty_id);
FL_EXTERN void ty_dump_cell(size_t ty_id, int indent);
FL_EXTERN void ty_dump_table();

/* cldoc:end-category() */

/* cldoc:begin-category(typesystem/typesystem-alloc.c) */

extern ty_t* ts_type_table;

extern size_t ts_type_size_s;

extern ts_type_hash_t* ts_hashtable;

/* Intialize type system global variables
 * Do not call this directly use: [flang_init](#flang_init)
 */
FL_EXTERN void ts_init();

/* Free global variables
 * Do not call this directly use: [flang_exit](#flang_exit)
 */
FL_EXTERN void ts_exit();
/* cldoc:end-category() */

/* cldoc:begin-category(typesystem/typesystem.c) */

/* Check if given type is a struct
 *
 * @tyid type id
 * @return is a struct?
 */
FL_EXTERN bool ty_is_struct(size_t tyid);

/* Check if given type is a vector
 *
 * @tyid type id
 * @return is a vector?
 */
FL_EXTERN bool ty_is_vector(size_t tyid);

/* Check if given type is a integer or float
 *
 * @tyid type id
 * @return is a integer or float?
 */
FL_EXTERN bool ty_is_number(size_t tyid);

/* Check if given type is a floating point number
 *
 * @tyid type id
 * @return is a floating point number?
 */
FL_EXTERN bool ty_is_fp(size_t tyid);

/* Check if given type is a integer
 *
 * @tyid type id
 * @return is a integer?
 */
FL_EXTERN bool ty_is_int(size_t tyid);

/* Check if given type is a pointer
 *
 * @tyid type id
 * @return is a pointer?
 */
FL_EXTERN bool ty_is_pointer(size_t tyid);

/* Return how many pointer deep is the current non-pointer type
 *
 * examples: ptr(vector(ptr(i8))) is 1
 * examples: ptr(ptr(i8)) is 2
 *
 * @tyid type id
 * @return pointer count
 */
FL_EXTERN size_t ty_get_pointer_level(size_t tyid);

/* Check if given type is a function
 *
 * @tyid type id
 * @return is a function?
 */
FL_EXTERN bool ty_is_function(size_t tyid);

/* Create a wrapper type (ptr, vector, array, etc...)
 *
 * @wrapper_type wrapper type
 * @child_tyid child type id, can be a built-in or another wrapper
 * @return the type id (unique atm)
 */
FL_EXTERN size_t ty_create_wrapped(ts_types_t wrapper_type, size_t child_tyid);

/* Get the index in the struct of given property
 *
 * @tyid type id, must be a struct
 * @property property as text
 * @return -1 on error/not found, >= 0 otherwise
 */
FL_EXTERN size_t ty_get_struct_prop_idx(size_t tyid, string* property);

/* Get the index in the struct of given property
 *
 * @tyid type id, must be a struct
 * @property property as text
 * @return 0 on error/not found, > 0 otherwise
 */
FL_EXTERN size_t ty_get_struct_prop_type(size_t tyid, string* property);

/* Create a new type with given name.
 * This is used by functions and structs, so both types can collide
 * in name.
 * This is called after [ts_create_struct](#ts_create_struct) &
 * [ty_create_fn](#ty_create_fn)
 *
 * @id name
 * @decl ast declaration, cannot be null
 * @tyid type id
 * @return 0 on error/not found, > 0 otherwise
 */
FL_EXTERN void ty_create_named(string* id, ast_t* decl, size_t tyid);

/* Create a new type given a struct declaration.
 * Ensure uniqueness of the returned ty_id
 * If the type should be indexed (~public) use:
 *[ty_create_named](#ty_create_named)
 *
 * @decl struct declaration, cannot be null
 * @return type id
 */
FL_EXTERN size_t ty_create_struct(ast_t* decl);

/* Create a new type given a function declaration.
 * Ensure uniqueness of the returned ty_id
 * If the type should be indexed (~public) use:
 *[ty_create_named](#ty_create_named)
 *
 * @decl function declaration, cannot be null
 * @return type id
 */
FL_EXTERN size_t ty_create_fn(ast_t* decl);

/* Given the function name return it's ty_id
 * This function does not handle collisions, return the first
 *
 * @id function name
 * @return type id
 */
FL_EXTERN size_t ty_get_fn_typeid(ast_t* id);

/* Given a type name return the hash item
 *
 * @id type name
 * @return hash item or null
 */
FL_EXTERN ts_type_hash_t* ty_get_type_by_name(string* id);

/* Given an identifier literal, traverse the tree searching
 * all types. Raise an error if more than one type is found.
 *
 * TODO move it to ast-query.c?
 *
 * @id type name
 * @return type id, 0 if not found
 */
FL_EXTERN size_t ty_get_typeid_by_name(ast_t* node);

/* cldoc:end-category() */

// TODO continue documenting

/* cldoc:begin-category(typesystem/typesystem.c) */
FL_EXTERN size_t ts_promote_typeid(size_t a, size_t b);
FL_EXTERN ast_t* ts_pass(ast_t* node);
FL_EXTERN void ts_pass_try(ast_t* node);

// return the unique typeid given ret + arguments
FL_EXTERN size_t ty_create_fn(ast_t* decl);
// return the unique typeid given fields
FL_EXTERN size_t ty_create_struct(ast_t* decl);

FL_EXTERN size_t ast_get_struct_prop_idx(ast_t* decl, string* id);

FL_EXTERN ast_t* ast_search_fn_wargs(string* id, ast_t* args);
FL_EXTERN size_t ast_get_ident_typeid(ast_t* id);

/* cldoc:end-category() */

/* cldoc:begin-category(typesystem/type-cast.c) */
FL_EXTERN bool ts_castable(size_t aty_id, size_t bty_id);
FL_EXTERN ast_cast_operations_t ts_cast_operation(ast_t* node);
FL_EXTERN ast_action_t
ts_cast_operation_pass_cb(ast_t* node, ast_t* parent, size_t level,
                          void* userdata_in, void* userdata_out);

FL_EXTERN ast_t* ts_create_cast(ast_t* node, size_t type_id);
FL_EXTERN ast_t* ts_create_left_cast(ast_t* parent, ast_t* left);
FL_EXTERN ast_t* ts_create_right_cast(ast_t* parent, ast_t* right);
FL_EXTERN void ts_create_binop_cast(ast_t* bo);

FL_EXTERN void ts_cast_return(ast_t* node);
FL_EXTERN void ts_cast_lunary(ast_t* node);
FL_EXTERN void ts_cast_assignament(ast_t* node);
FL_EXTERN void ts_cast_call(ast_t* node);
FL_EXTERN void ts_cast_binop(ast_t* node);
FL_EXTERN void ts_cast_expr_member(ast_t* node);
/* cldoc:end-category() */

/* cldoc:begin-category(parser/load-imports.c) */
FL_EXTERN ast_t* ast_load_imports(ast_t* node);
/* cldoc:end-category() */

/* cldoc:begin-category(ast/ast.c) */

/**
 * @returns if the iteration must stop
 */
typedef ast_action_t (*ast_cb_t)(ast_t* node, ast_t* parent, size_t level,
                                 void* userdata_in, void* userdata_out);

FL_EXTERN void ast_parent(ast_t* root);

FL_EXTERN size_t ast_get_typeid(ast_t* node);

FL_EXTERN bool ast_is_pointer(ast_t* node);

FL_EXTERN size_t ast_ret_type(ast_t* node);

FL_EXTERN ast_t* ast_find_fn_decl(ast_t* identifier);

FL_EXTERN array* ast_search_fns(ast_t* node, string* id);

/* cldoc:end-category() */

/* cldoc:begin-category(ast/ast-search.c) */
FL_EXTERN ast_t* ast_search_id_decl(ast_t* node, string* name);
FL_EXTERN ast_t* ast_search_fn(ast_t* node, string* identifier, size_t* args,
                               size_t nargs, size_t ret, bool var_args);
/* cldoc:end-category() */

/* cldoc:begin-category(ast/traverse.c) */
FL_EXTERN void ast_traverse(ast_t* ast, ast_cb_t cb, ast_t* parent,
                            size_t level, void* userdata_in,
                            void* userdata_out);
FL_EXTERN void ast_traverse_list(ast_t* node, ast_cb_t cb, ast_t* until,
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

FL_EXTERN ast_t* ast_clone(ast_t* node);

/* cldoc:end-category() */

/* cldoc:begin-category(ast/ast-static.c) */
FL_EXTERN bool ast_is_static(ast_t* node);
/* cldoc:end-category() */

/* cldoc:begin-category(ast/ast-query.c) */
FL_EXTERN bool ast_is_literal(ast_t* node);
/* cldoc:end-category() */

/* cldoc:begin-category(ast/ast-codegen.c) */
FL_EXTERN bool ast_require_load(ast_t* node);
/* cldoc:end-category() */

/* cldoc:begin-category(ast/ast-error.c) */
extern char* ast_err_buff;
FL_EXTERN bool ast_print_error(ast_t* node);
FL_EXTERN void ast_raise_error(ast_t* node, char* message, ...);
/* cldoc:end-category() */

/* cldoc:begin-category(typesystem/typesystem-inference.c) */

FL_EXTERN ast_t* ts_inference(ast_t* node);

/* cldoc:end-category() */

/* cldoc:begin-category(typesystem/typesystem-register.c) */

FL_EXTERN ast_t* ts_register_types(ast_t* node);

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
FL_EXTERN LLVMValueRef cg_lit_integer(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef cg_lit_float(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef cg_lit_boolean(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef cg_lit_string(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef cg_assignament(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef cg_dtor_var(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef cg_function(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef cg_return(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef cg_expr_call(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef cg_runary(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef cg_lunary(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef cg_if(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef cg_loop(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef cg_left_identifier(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef cg_right_identifier(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef cg_right_member(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef cg_left_member(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef cg_lhs(FL_CODEGEN_HEADER);
FL_EXTERN LLVMValueRef cg_sizeof(FL_CODEGEN_HEADER);
/* cldoc:end-category() */

/* cldoc:begin-category(codegen-types.c) */
FL_EXTERN bool cg_bitcast(ty_t a, ty_t b);
FL_EXTERN LLVMTypeRef cg_get_type(ast_t* node, LLVMContextRef context);
FL_EXTERN LLVMTypeRef cg_get_typeid(size_t id, LLVMContextRef context);
/* cldoc:end-category() */

/* cldoc:begin-category(codegen-utils.c) */
FL_EXTERN void cg_utils_store(ast_t* identifier, LLVMValueRef value,
                              LLVMBuilderRef builder);
/* cldoc:end-category() */

/* cldoc:begin-category(debug.c) */
// 4 by default
extern int log_debug_level;
/* cldoc:end-category() */

void ast_position(ast_t* target, YYLTYPE start, YYLTYPE end);
ast_t* ast_new();
ast_t* ast_mk_program(ast_t* block);
ast_t* ast_mk_root();
ast_t* ast_mk_list();
ast_t* ast_mk_list_push(ast_t* list, ast_t* node);
ast_t* ast_mk_list_insert(ast_t* list, ast_t* node, size_t idx);
ast_t* ast_mk_insert_before(ast_t* list, ast_t* search_item,
                            ast_t* insert_item);
ast_t* ast_mk_block(ast_t* body);
ast_t* ast_mk_lit_id(string* str, bool resolve);
ast_t* ast_mk_lit_null();
ast_t* ast_mk_lit_string(char* str, bool interpolate);
ast_t* ast_mk_lit_boolean(bool value);
ast_t* ast_mk_return(ast_t* argument);
ast_t* ast_mk_var_decl(ast_t* type, ast_t* id);
ast_t* ast_mk_lit_integer(char* text);
ast_t* ast_mk_lit_float(char* text);
ast_t* ast_mk_binop(ast_t* left, int op, ast_t* right);
ast_t* ast_mk_fn_decl(ast_t* id, ast_t* params, ast_t* ret_type, ast_t* body);
ast_t* ast_mk_fn_param(ast_t* id, ast_t* type, ast_t* def);
ast_t* ast_mk_assignament(ast_t* left, int op, ast_t* right);
ast_t* ast_mk_call_expr(ast_t* callee, ast_t* arguments);
ast_t* ast_mk_type_void();
ast_t* ast_mk_type(string* id, ast_t* child);
ast_t* ast_mk_type2(ast_t* id, ast_t* child);
ast_t* ast_mk_comment(string* text);
ast_t* ast_mk_lunary(ast_t* element, int operator);
ast_t* ast_mk_runary(ast_t* element, int operator);
ast_t* ast_mk_if(ast_t* test, ast_t* block, ast_t* alternate);
ast_t* ast_mk_loop(ast_t* init, ast_t* pre_cond, ast_t* update, ast_t* block,
                   ast_t* post_cond);
ast_t* ast_mk_struct_decl(ast_t* id, ast_t* fields);
ast_t* ast_mk_struct_decl_field(ast_t* id, ast_t* type);
ast_t* ast_mk_break(ast_t* argument);
ast_t* ast_mk_continue(ast_t* argument);
ast_t* ast_mk_member(ast_t* left, ast_t* property, bool expression);
ast_t* ast_mk_cast(ast_t* type, ast_t* element);
ast_t* ast_mk_sizeof(ast_t* type);
ast_t* ast_mk_import(ast_t* string_lit);
ast_t* ast_mk_error(const char* message, char* type);

/* cldoc:begin-category(typesystem/typesystem.c) */
/** Pass all typesystem to given ast_t
 * First it will fetch all imports
 * Then register all types
 * Then apply inference and casting logics
 */
FL_EXTERN ast_t* typesystem(ast_t* root);
/* cldoc:end-category() */
