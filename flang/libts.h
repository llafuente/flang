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

extern ty_t* ts_type_table;
extern u64 ts_type_size_s;

#define TS_VOID 1
#define TS_BOOL 2
#define TS_U8 3
#define TS_I8 4
#define TS_U16 5
#define TS_I16 6
#define TS_U32 7
#define TS_I32 8
#define TS_U64 9
#define TS_I64 10
#define TS_F32 11
#define TS_F64 12
#define TS_CSTR 13
#define TS_PVOID 14
#define TS_VECI8 15
#define TS_STRING 13 // TODO this should be 16

// DECL

enum ts_types {
  FL_VOID = 1,
  FL_NUMBER = 2,
  FL_POINTER = 3, // wrapper
  FL_VECTOR = 4,  // wrapper
  FL_FUNCTION = 5,
  FL_STRUCT = 6,
  // FL_ENUM = 7, // TODO this is in fact an "int"

  FL_INFER = 10,

  FL_TEMPLATE = 20,
};

// type must be unique
struct ts_type {
  string* id;
  string* decl;
  string* cg;

  ts_types_t of;
  void* codegen; // cache for codegen.

  // string, array, any are implemented inside the language
  // ref must be studied where should be, it's a very special pointer...
  union {
    // void is void :)

    // bool, iX, uX, fX
    struct ts_type_number {
      unsigned char bits;
      bool fp;
      bool sign;
    } number;

    struct ts_type_pointer {
      u64 to;
    } ptr;

    struct ts_type_vector {
      u64 to;
      u64 length;
    } vector;

    struct ts_type_function {
      ast_t* decl;

      u64 ret;
      u64* params;
      u64 nparams;
      bool varargs;

    } func;

    struct ts_type_struct {
      ast_t* decl;

      u64* fields;
      string** properties;
      u64 nfields;

    } structure;

    struct ts_type_enum {
      ast_t* decl;

      u64* members;
      u64 nmembers;

    } enu;

    struct ts_type_template {
      ast_t* decl;
    } tpl;
  };
};


/* cldoc:begin-category(type-dump.c) */
/* Get default colors for debugging
 *
 * @ty_id type id
 * @return ansi color escape code token
 */
char* ty_to_color(u64 ty_id);

/* Get printf token given ty_id, only built-in atm.
 *
 * @ty_id type id
 * @return printf token
 */
libexport char* ty_to_printf(u64 ty_id);

/* Get string representation of given type.
 * Do not need to free ther string, it's allocated in the pool.
 *
 * @ty_id type id
 * @return type as string
 */
libexport string* ty_to_string(u64 ty_id);

/* Print string representation to stderr
 *
 * @ty_id type id
 */
libexport void ty_dump(u64 ty_id);
/* Print a table with all type registered
 */
libexport void ty_dump_table();

/* cldoc:end-category() */

/* cldoc:begin-category(type-utils.c) */

/* Check if given type is a struct
 *
 * @tyid type id
 * @return is a struct?
 */
libexport bool ty_is_struct(u64 tyid);

/* Check if given type is a vector
 *
 * @tyid type id
 * @return is a vector?
 */
libexport bool ty_is_vector(u64 tyid);

/* Check if given type is a integer or float
 *
 * @tyid type id
 * @return is a integer or float?
 */
libexport bool ty_is_number(u64 tyid);

/* Check if given type is a floating point number
 *
 * @tyid type id
 * @return is a floating point number?
 */
libexport bool ty_is_fp(u64 tyid);

/* Check if given type is a integer
 *
 * @tyid type id
 * @return is a integer?
 */
libexport bool ty_is_int(u64 tyid);

/* Check if given type is a pointer
 *
 * @tyid type id
 * @return is a pointer?
 */
libexport bool ty_is_pointer(u64 tyid);

/* Check if given type is a template
 *
 * @tyid type id
 * @return is a pointer?
 */
libexport bool ty_is_template(u64 tyid);

/* Return how many pointer deep is the current non-pointer type
 *
 * examples: ptr(vector(ptr(i8))) is 1
 * examples: ptr(ptr(i8)) is 2
 *
 * @tyid type id
 * @return pointer count
 */
libexport u64 ty_get_pointer_level(u64 tyid);

/* Check if given type is a function
 *
 * @tyid type id
 * @return is a function?
 */
libexport bool ty_is_function(u64 tyid);

/* Create a wrapper type (ptr, vector, array, etc...)
 *
 * @wrapper_type wrapper type
 * @child_tyid child type id, can be a built-in or another wrapper
 * @return the type id (unique atm)
 */
libexport u64 ty_create_wrapped(ts_types_t wrapper_type, u64 child_tyid);

/* Get the index in the struct of given property
 *
 * @tyid type id, must be a struct
 * @property property as text
 * @return -1 on error/not found, >= 0 otherwise
 */
libexport u64 ty_get_struct_prop_idx(u64 tyid, string* property);

/* Get the index in the struct of given property
 *
 * @tyid type id, must be a struct
 * @property property as text
 * @return 0 on error/not found, > 0 otherwise
 */
libexport u64 ty_get_struct_prop_type(u64 tyid, string* property);

/* Create a new type given a struct declaration.
 * If the type should be indexed (~public) use:
 *
 * @decl struct declaration, cannot be null
 * @return type id
 */
libexport u64 ty_create_struct(ast_t* decl);

/* Check if both structs are compatible/castable
 *
 * @a
 * @b
 * @return compatible
 */
libexport bool ty_compatible_struct(u64 a, u64 b);

/* Check if an argument list is compatible with given type
 *
 * *Note* remember that templates cannot be casted!
 *
 * @a
 * @arg_list
 * @strict (true) types must be the same (false) type can be casted
 * @template if a type is a template skip
 * @return compatible
 */
libexport bool ty_compatible_fn(u64 a, ast_t* arg_list, bool strict,
                                bool template);

/* Create a new type given a function declaration.
 * Ensure uniqueness of the returned ty_id
 * If the type should be indexed (~public) use:
 *
 * @decl function declaration, cannot be null
 * @return type id
 */
libexport u64 ty_create_fn(ast_t* decl);

/* Register variable in desired scope
 *
 * @decl
 */
libexport void ty_create_var(ast_t* decl);

libexport u64 ty_create_template(ast_t* decl);

/* cldoc:end-category() */

/* cldoc:begin-category(typesystem-alloc.c) */

extern ty_t* ts_type_table;

extern u64 ts_type_size_s;

/* Intialize type system global variables
 * Do not call this directly use: [flang_init](#flang_init)
 */
libexport void ts_init();

/* Free global variables
 * Do not call this directly use: [flang_exit](#flang_exit)
 */
libexport void ts_exit();
/* cldoc:end-category() */

// TODO continue documenting

/* cldoc:begin-category(typesystem-promotion.c) */

/* Return if current can be safetly casted to expected
 *
 * @current type id
 * @expected type id
 */
libexport bool ts_castable(u64 current, u64 expected);

/* Given a AST_CAST node, will determine the asm operation
 * needed to cast both types.
 * If no operation is found, it will try to search for an
 * appropiate `autocast` function or raise otherwise.
 *
 * @node AST_CAST node
 */
libexport ast_cast_operations_t ts_cast_operation(ast_t* node);

/* Try to cast a literal to the given type_id
 *
 * @node node literal
 * @type_id type id
 */
libexport bool ts_cast_literal(ast_t* node, u64 type_id);

/* Check if a return statement need to be casted
 * @node node
 */
libexport void ts_cast_return(ast_t* node);

/* Check if a left unary expression need to be casted
 * @node node
 */
libexport void ts_cast_lunary(ast_t* node);

/* Check if an assignament expression need to be casted
 * @node node
 */
libexport void ts_cast_assignament(ast_t* node);

/* Check if an expression call need to be casted
 * @node node
 */
libexport void ts_cast_call(ast_t* node);

/* Check if an binary operation expression need to be casted
 * either right or left.
 * @node node
 */
libexport void ts_cast_binop(ast_t* node);

/* Check if an meber expression need to be casted
 * @node node
 */
libexport void ts_cast_expr_member(ast_t* node);

/* cldoc:end-category() */

/* cldoc:begin-category(typesystem-inference.c) */

/* Traverse the node and try to figure the type of all
 * declarations. This is called many times to try to resolve
 * dependencies.
 * @node node
 */
libexport ast_t* ts_inference(ast_t* node);

/* cldoc:end-category() */

/* cldoc:begin-category(typesystem-pass.c) */

/* Traverse the tree enforcing types. After this, all the tree
 * has types and can be codegen.
 * @node node
 */
libexport ast_t* ts_pass(ast_t* node);

/* cldoc:end-category() */

/* cldoc:begin-category(typesystem-promotion.c) */

/* `a` can be promoted to `b`
 * Only numbers can be promoted.
 *
 * @a source type id
 * @a destination type id
 * @return type id, 0 if not found
 */
libexport u64 ts_promote_typeid(u64 a, u64 b);

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
libexport ast_t* ts_register_types(ast_t* node);

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
libexport ast_t* typesystem(ast_t* root);

/* cldoc:end-category() */
