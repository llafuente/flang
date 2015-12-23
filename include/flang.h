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

#ifndef ASAN
#error : ASAN must be either 0 or 1, use -DASAN=X
#endif

extern void __sanitizer_print_stack_trace();

#if ASAN == 1
#undef NDEBUG
#define assert(check)                                                          \
  {                                                                            \
    fprintf("assert check: %s\n", #check);                                     \
    if (!(check)) {                                                            \
      fprintf("assert failed: %s\n", #check);                                  \
      __sanitizer_print_stack_trace();                                         \
    }                                                                          \
  }

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

#include "ext/array.h"
#include "ext/pool.h"
#include "ext/hash.h"

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

// callback type for [ast_traverse](#ast_traverse) & [ast_reverse](#ast_reverse)
typedef ast_action_t (*ast_cb_t)(ast_t* node, ast_t* parent, size_t level,
                                 void* userdata_in, void* userdata_out);

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
extern char* ast_last_error_message;
extern ast_t* ast_last_error_node;

/* Initiaze global variables and memory pool.
 * Must be called before anything else.
 *
 * see: [flang_exit](#flang_exit)
 */
FL_EXTERN void flang_init();

/* Free global variables and the memory pool
 * see: [flang_init](#flang_init)
 * @root node
 */
FL_EXTERN void flang_exit(ast_t* root);

/* cldoc:end-category() */

/* cldoc:begin-category(parser-utils.c) */

/* Parse an utf8 string and return the root ast node.
 *
 * see: [fl_parse_file](#fl_parse_file)
 *
 * @str input string
 * @return root ast node
 */
FL_EXTERN ast_t* fl_parse_utf8(char* str);

/* Parse an utf8 string and return the root ast node
 * with import core at the beginning. So you can later call
 * ast_load_imports and get the full ast.
 *
 * see: [fl_parse_main_file](#fl_parse_main_file)
 *
 * @str input string
 * @return root ast node
 */
FL_EXTERN ast_t* fl_parse_main_utf8(char* str);

/* Return file contents parsed.
 *
 * see: [fl_parse_utf8](#fl_parse_utf8)
 *
 * @filename path to file
 * @return root ast node
 */
FL_EXTERN ast_t* fl_parse_file(const char* filename);

/* Return file contents parsed with import core at the beginning.
 * So you can later call ast_load_imports and get the full ast.
 *
 * see: [fl_parse_main_utf8](#fl_parse_main_utf8)
 *
 * @filename path to file
 * @return root ast node
 */
FL_EXTERN ast_t* fl_parse_main_file(const char* filename);

/* Help function, exported to easy test code.
 *
 * @filename path to file
 * @return file contents as string
 */
FL_EXTERN string* fl_file_to_string(const char* filename);

/* cldoc:end-category() */

/* cldoc:begin-category(type-dump.c) */

/* Get printf token given ty_id, only built-in atm.
 *
 * @ty_id type id
 * @return printf token
 */
FL_EXTERN char* ty_to_printf(size_t ty_id);

/* Get string representation of given type.
 * Do not need to free ther string, it's allocated in the pool.
 *
 * @ty_id type id
 * @return type as string
 */
FL_EXTERN string* ty_to_string(size_t ty_id);

/* Print string representation to stderr
 *
 * @ty_id type id
 */
FL_EXTERN void ty_dump(size_t ty_id);
/* Print a table with all type registered
 */
FL_EXTERN void ty_dump_table();

/* cldoc:end-category() */

/* cldoc:begin-category(type-utils.c) */

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
 * If the type should be indexed (~public) use:
 *[ty_create_named](#ty_create_named)
 *
 * @decl struct declaration, cannot be null
 * @return type id
 */
FL_EXTERN size_t ty_create_struct(ast_t* decl);

/* Check if both structs are compatible/castable
 *
 * @a
 * @b
 * @return type id
 */
FL_EXTERN bool ty_compatible_struct(size_t a, size_t b);

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

/* Register variable in desired scope
 *
 * @decl
 */
FL_EXTERN void ty_create_var(ast_t* decl);

/* cldoc:end-category() */

/* cldoc:begin-category(typesystem-alloc.c) */

extern ty_t* ts_type_table;

extern size_t ts_type_size_s;

/* Intialize type system global variables
 * Do not call this directly use: [flang_init](#flang_init)
 */
FL_EXTERN void ts_init();

/* Free global variables
 * Do not call this directly use: [flang_exit](#flang_exit)
 */
FL_EXTERN void ts_exit();
/* cldoc:end-category() */

// TODO continue documenting

/* cldoc:begin-category(typesystem-promotion.c) */

/* Return if current can be safetly casted to expected
 *
 * @current type id
 * @expected type id
 */
FL_EXTERN bool ts_castable(size_t current, size_t expected);

/* Given a FL_AST_CAST node, will determine the asm operation
 * needed to cast both types.
 * If no operation is found, it will try to search for an
 * appropiate `autocast` function or raise otherwise.
 *
 * @node FL_AST_CAST node
 */
FL_EXTERN ast_cast_operations_t ts_cast_operation(ast_t* node);

/* Try to cast a literal to the given type_id
 *
 * @node node literal
 * @type_id type id
 */
FL_EXTERN bool ts_cast_literal(ast_t* node, size_t type_id);

/* Check if a return statement need to be casted
 * @node node
 */
FL_EXTERN void ts_cast_return(ast_t* node);

/* Check if a left unary expression need to be casted
 * @node node
 */
FL_EXTERN void ts_cast_lunary(ast_t* node);

/* Check if an assignament expression need to be casted
 * @node node
 */
FL_EXTERN void ts_cast_assignament(ast_t* node);

/* Check if an expression call need to be casted
 * @node node
 */
FL_EXTERN void ts_cast_call(ast_t* node);

/* Check if an binary operation expression need to be casted
 * either right or left.
 * @node node
 */
FL_EXTERN void ts_cast_binop(ast_t* node);

/* Check if an meber expression need to be casted
 * @node node
 */
FL_EXTERN void ts_cast_expr_member(ast_t* node);

/* cldoc:end-category() */

/* cldoc:begin-category(typesystem-inference.c) */

/* Traverse the node and try to figure the type of all
 * declarations. This is called many times to try to resolve
 * dependencies.
 * @node node
 */
FL_EXTERN ast_t* ts_inference(ast_t* node);

/* cldoc:end-category() */

/* cldoc:begin-category(typesystem-pass.c) */

/* Traverse the tree enforcing types. After this, all the tree
 * has types and can be codegen.
 * @node node
 */
FL_EXTERN ast_t* ts_pass(ast_t* node);

/* cldoc:end-category() */

/* cldoc:begin-category(typesystem-promotion.c) */

/* `a` can be promoted to `b`
 * Only numbers can be promoted.
 *
 * @a source type id
 * @a destination type id
 * @return type id, 0 if not found
 */
FL_EXTERN size_t ts_promote_typeid(size_t a, size_t b);

/* cldoc:end-category() */

/* cldoc:begin-category(typesystem-register.c) */

/* Traverse the tree transforming all type nodes (text)
 * to it's internal tyid (number) representation.
 * Structs and functions are also typed.
 *
 * @a source type id
 * @a destination type id
 * @return type id, 0 if not found
 */
FL_EXTERN ast_t* ts_register_types(ast_t* node);

/* cldoc:end-category() */

/* cldoc:begin-category(typesystem.c) */

/* This will transform the raw tree parsed by
 * flex/bison to a good codegen friendly tree.
 * * Inference types
 * * Promote types
 * * Implicit casting
 * * Handle autocast functions
 *
 * TODO
 * * Function operators (are like casting)
 */
FL_EXTERN ast_t* typesystem(ast_t* root);

/* cldoc:end-category() */

// - codegen

/* cldoc:begin-category(codegen-export.c) */

/* Export to bitcode given module.
 *
 * @module Module to export
 * @filename export filename (path must exits)
 * @return true if everything goes ok
 */
FL_EXTERN bool fl_to_bitcode(LLVMModuleRef module, const char* filename);

/* Export to IR given module.
 *
 * @module Module to export
 * @filename export filename (path must exits)
 * @return true if everything goes ok
 */
FL_EXTERN bool fl_to_ir(LLVMModuleRef module, const char* filename);

/* cldoc:end-category() */

/* cldoc:begin-category(codegen-interpreter.c) */

/* Execute main function of given module.
 *
 * @module Module to execute
 */
FL_EXTERN void fl_interpreter(LLVMModuleRef module);

/* cldoc:end-category() */

// - ast/*

/* cldoc:begin-category(ast-alloc.c) */

/* Create a new AST from the pool.
 *
 * @return new ast (do not free it yourself)
 */
FL_EXTERN ast_t* ast_new();

/* Free non-pool memory asociated with the node
 *
 * @node
 */
FL_EXTERN void ast_delete(ast_t* node);

/* Clone a node
*
 * TODO not all nodes are supported, check code before!
 *
 * @node
 * @return cloned node (do not free it yourself)
 */
FL_EXTERN ast_t* ast_clone(ast_t* node);

/* cldoc:end-category() */

/* cldoc:begin-category(ast-codegen.c) */

/* Determine if given node require to be loaded to access it value
 * @node
 * @return true if load is required
 */
FL_EXTERN bool ast_require_load(ast_t* node);

/* cldoc:end-category() */

/* cldoc:begin-category(ast-codegen.c) */

/* Debug: Print to stderr a node as text
 * @node
 */
FL_EXTERN void ast_dump_one(ast_t* node);

/* Debug: Recursive print to stderr a node as text
 * @node
 */
FL_EXTERN void ast_dump(ast_t* node);

/* Debug: Recursive print to stderr a node as text (do not follow imports)
 * @node
 */
FL_EXTERN void ast_mindump(ast_t* node);

/* cldoc:end-category() */

/* cldoc:begin-category(ast-error.c) */

/* Print an error to stderr
 * @node
 */
FL_EXTERN bool ast_print_error(ast_t* node);

/* Print an error an trace to stderr, then exit
 * @node
 * @message
 */
FL_EXTERN void ast_print_error_at(ast_t* node, char* message);

/* Print an error an trace to stderr, then exit
 * @node
 * @message
 */
FL_EXTERN void ast_raise_error(ast_t* node, char* message, ...);

/* cldoc:end-category() */

/* cldoc:begin-category(ast-get.c) */

/* Get tyid from given AST Identifier
 * @id Identifier node
 * @return type id
 */
FL_EXTERN size_t ast_get_ident_typeid(ast_t* id);

/* Get attribute with given id
 * @list list of attributes
 * @string attribute id
 * @return node if found, 0 otherwise
 */
FL_EXTERN ast_t* ast_get_attribute(ast_t* list, string* needle);

/* Reverse the tree a get the global scope
 * @node
 * @return node if found, 0 otherwise
 */
FL_EXTERN ast_t* ast_get_global_scope(ast_t* node);

/* Reverse the tree searching nearest scope
 * @node
 * @return neasert scope
 */
FL_EXTERN ast_t* ast_get_scope(ast_t* node);

/* Get location as string
 * @node
 * @return string location
 */
FL_EXTERN string* ast_get_location(ast_t* node);

/* cldoc:end-category() */

/* cldoc:begin-category(ast-load-imports.c) */

/* Traverse the tree loading and appending all imports.
 * Imports cannot be double loaded so it safe.
 * @node
 */
FL_EXTERN ast_t* ast_load_imports(ast_t* node);

/* cldoc:end-category() */

/* cldoc:begin-category(ast-mk.c) */
//- self explanatory
FL_EXTERN void ast_position(ast_t* target, YYLTYPE start, YYLTYPE end);
FL_EXTERN ast_t* ast_mk_program(ast_t* block);
FL_EXTERN ast_t* ast_mk_error(const char* message, char* type);
FL_EXTERN ast_t* ast_mk_list();
FL_EXTERN ast_t* ast_mk_list_push(ast_t* list, ast_t* node);
FL_EXTERN ast_t* ast_mk_list_insert(ast_t* list, ast_t* node, size_t idx);
FL_EXTERN ast_t* ast_mk_insert_before(ast_t* list, ast_t* search_item,
                                      ast_t* insert_item);
FL_EXTERN ast_t* ast_mk_block(ast_t* body);
FL_EXTERN ast_t* ast_mk_lit_id(string* str, bool resolve);
FL_EXTERN ast_t* ast_mk_lit_null();
FL_EXTERN ast_t* ast_mk_lit_string(char* str, bool interpolate);
FL_EXTERN ast_t* ast_mk_lit_string2(string* str, bool interpolate, bool escape);
FL_EXTERN ast_t* ast_mk_lit_boolean(bool value);
FL_EXTERN ast_t* ast_mk_lit_integer(char* text);
FL_EXTERN ast_t* ast_mk_lit_float(char* text);
FL_EXTERN ast_t* ast_mk_return(ast_t* argument);
FL_EXTERN ast_t* ast_mk_break(ast_t* argument);
FL_EXTERN ast_t* ast_mk_continue(ast_t* argument);
FL_EXTERN ast_t* ast_mk_var_decl(ast_t* type, ast_t* id, ast_scope_t scope);
FL_EXTERN ast_t* ast_mk_fn_decl(ast_t* id, ast_t* params, ast_t* ret_type,
                                ast_t* body, ast_t* attibutes);
FL_EXTERN void ast_mk_fn_decl_body(ast_t* fn, ast_t* body);
FL_EXTERN ast_t* ast_mk_fn_param(ast_t* id, ast_t* type, ast_t* def);
FL_EXTERN ast_t* ast_mk_binop(ast_t* left, int op, ast_t* right);
FL_EXTERN ast_t* ast_mk_assignament(ast_t* left, int op, ast_t* right);
FL_EXTERN ast_t* ast_mk_call_expr(ast_t* callee, ast_t* arguments);
FL_EXTERN ast_t* ast_mk_type_auto();
FL_EXTERN ast_t* ast_mk_type_void();
FL_EXTERN ast_t* ast_mk_type(string* id, ast_t* child);
FL_EXTERN ast_t* ast_mk_type2(ast_t* id, ast_t* child);
FL_EXTERN ast_t* ast_mk_comment(string* text);
FL_EXTERN ast_t* ast_mk_lunary(ast_t* element, int operator);
FL_EXTERN ast_t* ast_mk_runary(ast_t* element, int operator);
FL_EXTERN ast_t* ast_mk_if(ast_t* test, ast_t* block, ast_t* alternate);
FL_EXTERN ast_t* ast_mk_loop(ast_t* init, ast_t* pre_cond, ast_t* update,
                             ast_t* bloc, ast_t* post_cond);
FL_EXTERN ast_t* ast_mk_struct_decl(ast_t* id, ast_t* fields);
FL_EXTERN ast_t* ast_mk_struct_decl_field(ast_t* id, ast_t* type);
FL_EXTERN ast_t* ast_mk_member(ast_t* left, ast_t* property, bool expression);
FL_EXTERN ast_t* ast_mk_sizeof(ast_t* type);
FL_EXTERN ast_t* ast_mk_cast(ast_t* type, ast_t* element);
FL_EXTERN ast_t* ast_mk_import(ast_t* string_lit, bool foward);
FL_EXTERN ast_t* ast_mk_log(ast_t* list);
FL_EXTERN ast_t* ast_mk_attribute(ast_t* id, ast_t* value);

/* cldoc:end-category() */

/* cldoc:begin-category(ast-query.c) */

/* Return if the node is a literal
 * Note: lunary + number is a literal (-8)
 * Note: binop (number + number) is a literal (8 + 7)
 * Etc...
 * @node
 * @return if @node is a literal
 */
FL_EXTERN bool ast_is_literal(ast_t* node);

/* cldoc:end-category() */

/* cldoc:begin-category(ast-reduce.c) */

/* Reduce some tree nodes, so the codegen isn't redundant
 *
 * @node should be root
 * @return node o error
 */
FL_EXTERN ast_t* ast_reduce(ast_t* node);

/* cldoc:end-category() */

/* cldoc:begin-category(ast-reverse.c) */

/* Reverse the tree calling cb on each node.
 * [typesystem](#typesystem) set parent, so this cannot be called before
 *
 * @node
 * @cb
 * @parent
 * @level
 * @userdata_in
 * @userdata_out
 */
FL_EXTERN void ast_reverse(ast_t* node, ast_cb_t cb, ast_t* parent,
                           size_t level, void* userdata_in, void* userdata_out);
/* Return root node
 * A root node is the program or the module
 *
 * @node
 * @return root node
 */
FL_EXTERN ast_t* ast_get_root(ast_t* node);
/* Get the string piece of code given a node
 * Does not always work, because some node are auto-generated...
 *
 * @node
 * @return root node
 */
FL_EXTERN string* ast_get_code(ast_t* node);

/* cldoc:end-category() */

/* cldoc:begin-category(ast-search.c) */

/* From give node reverse the tree searching given identifier
 * @node
 * @identifier
 */
FL_EXTERN ast_t* ast_search_id_decl(ast_t* node, string* identifier);

/* From give node reverse the tree search searching given function
 * prototype
 * @node
 * @identifier
 * @args
 * @nargs
 * @ret_ty
 * @var_args
 */
FL_EXTERN ast_t* ast_search_fn(ast_t* node, string* identifier, size_t* args,
                               size_t nargs, size_t ret_ty, bool var_args);

/* Search a matching function prototype given "expression call"
 * @id
 * @args_call
 */
FL_EXTERN ast_t* ast_search_fn_wargs(string* id, ast_t* args_call);

/* Search all function with given name
 * @node
 * @id
 */
FL_EXTERN array* ast_search_fns(ast_t* node, string* id);

/* Reverse the tree and return first function found with given name
 * @identifier
 */
FL_EXTERN ast_t* ast_search_fn_decl(ast_t* identifier);

/* Traverse given tree and return all aparences of t
 * @node
 * @t
 * @return array or null
 */
FL_EXTERN array* ast_search_node_type(ast_t* node, ast_types_t t);

/* cldoc:end-category() */

/* cldoc:begin-category(ast-static.c) */

/* Return if the node is really a literal only
 * TODO review if logic is duplicated by [ast_id_literal](#ast_id_literal)
 * @node
 * @id
 */
FL_EXTERN bool ast_is_static(ast_t* node);

/* cldoc:end-category() */

/* cldoc:begin-category(ast-traverse.c) */

/* Traverse the tree calling cb on each node
 * @node
 * @cb
 * @parent
 * @level
 * @userdata_in
 * @userdata_out
 */
FL_EXTERN void ast_traverse(ast_t* node, ast_cb_t cb, ast_t* parent,
                            size_t level, void* userdata_in,
                            void* userdata_out);

/* Traverse a list calling cb on each node
 * @node
 * @cb
 * @parent
 * @level
 * @userdata_in
 * @userdata_out
*/
FL_EXTERN void ast_traverse_list(ast_t* node, ast_cb_t cb, ast_t* until,
                                 size_t level, void* userdata_in,
                                 void* userdata_out);

/* cldoc:end-category() */

/* cldoc:begin-category(ast.c) */

/* Traverse the tree and set parent
 * @root
 */
FL_EXTERN void ast_parent(ast_t* root);

/* Get tyid given a node
 * TODO this should be removed, in pro of just `node->ty_id`
 * @node
 */
FL_EXTERN size_t ast_get_typeid(ast_t* node);

/* Return if the type of node is a pointer (at first level!)
 * @node
 */
FL_EXTERN bool ast_is_pointer(ast_t* node);

/* cldoc:end-category() */

//- codegen/*

/* cldoc:begin-category(codegen-types.c) */

/* Return if it's possible to bitcast given types
 * This functions may need to be called twice swaping it's args
 * to be sure...
 *
 * @a
 * @b
 */
FL_EXTERN bool cg_bitcast(ty_t a, ty_t b);

/* Codegen the type and return it
 *
 * @node
 * @context
 */
FL_EXTERN LLVMTypeRef cg_get_type(ast_t* node, LLVMContextRef context);

/* Codegen the type and return it
 *
 * @tyid
 * @context
 */
FL_EXTERN LLVMTypeRef
cg_get_typeid(ast_t* node, size_t tyid, LLVMContextRef context);

/* cldoc:end-category() */

/* cldoc:begin-category(codegen-utils.c) */

/* Store a value into the identifier declaration
 *
 * @node must be an identifier
 * @value value to store
 * @builder current LLVMBuilderRef
 */
FL_EXTERN void cg_utils_store(ast_t* node, LLVMValueRef value,
                              LLVMBuilderRef builder);

/* cldoc:end-category() */

/* cldoc:begin-category(codegen.c) */

/* Create a single (atm) module with the given tree
 * It's use LLVM as backend.
 * @root Node
 * @module_name Module name
 */
FL_EXTERN LLVMModuleRef fl_codegen(ast_t* root, char* module_name);

/* cldoc:end-category() */
