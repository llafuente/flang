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

#include "src/libparser/grammar/parser.h"

// DECL

enum ast_action { AST_SEARCH_STOP = 0, AST_SEARCH_CONTINUE, AST_SEARCH_SKIP };
enum ast_trav_mode { AST_TRAV_ENTER = 0, AST_TRAV_LEAVE };

enum ast_function_type {
  AST_FUNC_FUNCTION = 1,
  AST_FUNC_OPERATOR,
  AST_FUNC_PROPERTY,
  AST_FUNC_CAST,
  AST_FUNC_ALIAS, // TODO doit!
};
enum ast_cast_operations {
  AST_CAST_ERR = 0, // unkown
  AST_CAST_FPTOSI,  // LLVMBuildFPToSI
  AST_CAST_FPTOUI,  // LLVMBuildFPToUI
  AST_CAST_SITOFP,  // LLVMBuildSIToFP
  AST_CAST_UITOFP,  // LLVMBuildUIToFP
  AST_CAST_FPEXT,   // LLVMBuildFPExt
  AST_CAST_SEXT,    // LLVMBuildSExt
  AST_CAST_ZEXT,    // LLVMBuildZExt
  AST_CAST_FPTRUNC, // LLVMBuildFPTrunc
  AST_CAST_TRUNC,   // LLVMBuildTrunc
  AST_CAST_BITCAST, // LLVMBuildBitCast
  AST_CAST_AUTO,    // Function call
};

enum ast_scope {
  AST_SCOPE_GLOBAL = 1,
  AST_SCOPE_BLOCK,
  AST_SCOPE_FUNCTION,
  AST_SCOPE_TRANSPARENT
};

enum ast_types {
  AST_PROGRAM = 1,
  AST_IMPORT = 2,
  AST_MODULE = 3,
  AST_BLOCK = 4,
  AST_LIST = 5,

  AST_LIT_ARRAY = 10,
  AST_LIT_OBJECT = 11,
  AST_LIT_INTEGER = 12,
  AST_LIT_FLOAT = 13,
  AST_LIT_STRING = 14,
  AST_LIT_BOOLEAN = 15,
  AST_LIT_NULL = 16,
  AST_LIT_IDENTIFIER = 17,

  AST_EXPR = 20,
  AST_EXPR_ASSIGNAMENT = 21,
  AST_EXPR_CONDITIONAL = 22,
  AST_EXPR_BINOP = 23,
  AST_EXPR_LUNARY = 24,
  AST_EXPR_RUNARY = 25,
  AST_EXPR_CALL = 26,
  AST_EXPR_MEMBER = 27,
  AST_EXPR_SIZEOF = 28,
  AST_EXPR_TYPEOF = 29,

  AST_CAST = 30,

  // TODO AST_DECL_VAR = 30
  AST_DTOR_VAR = 31,

  AST_TYPE = 40,
  AST_DECL_STRUCT = 41,
  AST_DECL_STRUCT_FIELD = 42,
  AST_DECL_STRUCT_ALIAS = 43,
  AST_DECL_TEMPLATE = 44,
  AST_IMPLEMENT = 45,

  AST_DECL_FUNCTION = 50,
  AST_PARAMETER = 51,
  AST_STMT_RETURN = 52,
  AST_STMT_BREAK = 53,
  AST_STMT_CONTINUE = 54,

  AST_STMT_IF = 60,
  AST_STMT_LOOP = 61,
  AST_STMT_FOR = 62,     // not used
  AST_STMT_WHILE = 63,   // not used
  AST_STMT_DOWHILE = 64, // not used

  AST_STMT_COMMENT = 100,
  AST_STMT_LOG = 101,
  AST_ATTRIBUTE = 102,

  AST_NEW = 110,
  AST_DELETE = 111,

  AST_ERROR = 255
};

struct ast {
  u64 id;

  u32 first_line;
  u32 first_column;
  u32 last_line;
  u32 last_column;

  ast_types_t type;
  ast_t* parent;

  u64 ty_id;

  // codegen
  bool dirty;

  u64 stack;

  u64 ts_passes;

  union {
    struct ast_error {
      string* message;
      string* type;
    } err;

    struct ast_program {
      string* path;
      // code as string
      string* code;
      string* file;
      // all types found in the code
      ty_t* types;
      u64 ntypes;
      // actual code parsed
      ast_t* body;
    } program;

    struct ast_block {
      // list of statements
      ast_t* body;

      ast_t* parent_scope;
      array modules; // list of modules

      ast_scope_t scope;

      hash_t* types;     // u64 with the type id
      hash_t* functions; // TODO array of decl?
      hash_t* variables; // ast_t* to declaration

      hash_t* uids; // bool, allocated by parser.y

    } block;

    struct ast_list {
      // count
      u64 length;
      u64 capacity;
      // list of statements
      ast_t** values;
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

    // NOTE: ast_expr_assignament & ast_expr_binop must be the same!
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
      bool unsafe;
    } cast;

    struct ast_dtor_variable {
      // TODO add type
      ast_t* id;
      ast_t* type;
      ast_scope_t scope;
      bool scoped; // a variable is attached to a scope?
    } var;

    struct ast_idtype {
      ast_t* id;
      ast_t* children;
    } ty;

    struct ast_decl_struct {
      ast_t* id;
      ast_t* fields; // list
      ast_t* tpls; // AST_LIST of ast_decl_struct_field or ast_decl_struct_alias

      ast_t* from_tpl; // cames from which template?
      bool templated;
    } structure; // aggregate

    struct ast_decl_struct_field {
      ast_t* type;
      ast_t* id;
    } field; // aggregate

    struct ast_decl_struct_alias {
      u64 index;
      ast_t* name;
      ast_t* id;
    } alias; // aggregate

    struct ast_decl_function {
      // TODO use ty_t*
      ast_t* id;
      string* uid; // final name
      ast_t* params;
      ast_t* body;
      ast_t* ret_type;
      ast_t* attributes;
      ast_t* from_tpl; // cames from which template?
      bool varargs;
      ast_function_type_t type;
      int operator;
      bool templated;
      bool ffi; // TODO maybe ffi_type, 0 means flang, 1 means c...
    } func;

    struct ast_decl_template {
      ast_t* id;
    } tpl;

    struct ast_implement {
      ast_t* type_id;
      ast_t* type_list;
      ast_t* uid;
    } impl;

    struct ast_parameter {
      ast_t* id;
      ast_t* type;
      ast_t* def;
      ast_t** assertions;
      u64 nassertions;
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
      // there are some sanity checks for user code that we don't for our
      // generated code, this bypass those checks
      bool safe_arguments;

      ast_t* decl; // set by typesystem
    } call;

    struct ast_expr_member {
      ast_t* left;
      ast_t* property;
      bool expression;
      // true if brakets where used so -> access operator overloading
      // false means dot access -> no operator overloading
      bool brakets;

      u64 idx; // calc by typesystem
    } member;

    struct ast_expr_sizeof {
      ast_t* type;
    } sof;

    struct ast_expr_typeof {
      ast_t* expr;
    } tof;

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

    struct ast_new {
      ast_t* expr;
    } new;

    struct ast_delete {
      ast_t* expr;
    } delete;
  };
};

/*----------------------------------------------------------------------------*/
/* cldoc:begin-category(parser/ast/ast.c) */

/* Traverse the tree and set parent
 * @root
 */
libexport void ast_parent(ast_t* root);

/* cldoc:end-category() */
/*----------------------------------------------------------------------------*/
/* cldoc:begin-category(parser/ast/ast-alloc.c) */

/* Create a new empty ast_t from the pool.
 *
 * @return new ast_t (do not free it yourself)
 */
libexport ast_t* ast_new();

/* Free non-pool memory asociated of given node
 *
 * @node
 */
libexport void ast_delete(ast_t* node);

/* Clone a node recursively
 *
 * @node
 * @return cloned node (do not free it yourself)
 */
libexport ast_t* ast_clone(ast_t* node);

/* cldoc:end-category() */
/*----------------------------------------------------------------------------*/
/* cldoc:begin-category(ast-mk.c) */
//- self explanatory
libexport void ast_position(ast_t* target, YYLTYPE start, YYLTYPE end);
libexport ast_t* ast_mk_program(ast_t* block);
libexport ast_t* ast_mk_error(const char* message, char* type);
libexport ast_t* ast_mk_list();
libexport ast_t* ast_mk_list_push(ast_t* list, ast_t* node);
libexport ast_t* ast_mk_list_pop(ast_t* list);
libexport ast_t* ast_mk_list_insert(ast_t* list, ast_t* node, u64 idx);
libexport ast_t* ast_mk_insert_before(ast_t* list, ast_t* search_item,
                                      ast_t* insert_item);
libexport ast_t* ast_mk_block(ast_t* body);
libexport ast_t* ast_mk_lit_id(string* str, bool resolve);
libexport ast_t* ast_mk_lit_null();
libexport ast_t* ast_mk_lit_string(char* str, bool interpolate);
libexport ast_t* ast_mk_lit_string2(string* str, bool interpolate, bool escape);
libexport ast_t* ast_mk_lit_boolean(bool value);
libexport ast_t* ast_mk_lit_integer(char* text);
libexport ast_t* ast_mk_lit_float(char* text);
libexport ast_t* ast_mk_return(ast_t* argument);
libexport ast_t* ast_mk_break(ast_t* argument);
libexport ast_t* ast_mk_continue(ast_t* argument);
libexport ast_t* ast_mk_var_decl(ast_t* type, ast_t* id, ast_scope_t scope);
libexport ast_t* ast_mk_fn_decl(ast_t* id, ast_t* params, ast_t* ret_type,
                                ast_t* body, ast_t* attibutes, int operator,
                                ast_function_type_t type);
libexport void ast_mk_fn_decl_body(ast_t* fn, ast_t* body);
libexport void ast_mk_fn_decl_attributes(ast_t* fn, ast_t* attributes);
libexport ast_t* ast_mk_fn_param(ast_t* id, ast_t* type, ast_t* def);
libexport ast_t* ast_mk_binop(ast_t* left, int op, ast_t* right);
libexport ast_t* ast_mk_assignament(ast_t* left, int op, ast_t* right);
libexport ast_t* ast_mk_call_expr(ast_t* callee, ast_t* arguments);
libexport ast_t* ast_mk_template(ast_t* id, ast_t* block);
libexport ast_t* ast_mk_implement(ast_t* type_id, ast_t* type_list, ast_t* uid);
libexport ast_t* ast_mk_new(ast_t* expr);
libexport ast_t* ast_mk_delete(ast_t* expr);
libexport ast_t* ast_mk_type_auto();
libexport ast_t* ast_mk_type_void();
libexport ast_t* ast_mk_type_pvoid();
libexport ast_t* ast_mk_type(string* id, ast_t* child);
libexport ast_t* ast_mk_type2(ast_t* id, ast_t* child);
libexport ast_t* ast_mk_comment(string* text);
libexport ast_t* ast_mk_lunary(ast_t* element, int operator);
libexport ast_t* ast_mk_runary(ast_t* element, int operator);
libexport ast_t* ast_mk_if(ast_t* test, ast_t* block, ast_t* alternate);
libexport ast_t* ast_mk_loop(ast_types_t type, ast_t* init, ast_t* pre_cond,
                             ast_t* update, ast_t* bloc, ast_t* post_cond);
libexport ast_t* ast_mk_struct_decl(ast_t* id, ast_t* tpls, ast_t* fields);
libexport ast_t* ast_mk_struct_decl_field(ast_t* id, ast_t* type);
libexport ast_t* ast_mk_struct_decl_alias(ast_t* name, ast_t* id);
libexport ast_t* ast_mk_member(ast_t* left, ast_t* property, bool expression,
                               bool brakets);
libexport ast_t* ast_mk_sizeof(ast_t* type);
libexport ast_t* ast_mk_typeof(ast_t* expr);
libexport ast_t* ast_mk_cast(ast_t* type, ast_t* element, bool unsafe);
libexport ast_t* ast_mk_import(ast_t* string_lit, bool foward);
libexport ast_t* ast_mk_log(ast_t* list);
libexport ast_t* ast_mk_attribute(ast_t* id, ast_t* value);

/* cldoc:end-category() */
/*----------------------------------------------------------------------------*/
/* cldoc:begin-category(ast-search.c) */

/* From give node reverse the tree search searching given function
 * prototype
 * @node
 * @identifier
 * @args
 * @nargs
 * @ret_ty
 * @var_args
 */
libexport ast_t* ast_search_fn(ast_t* node, string* identifier, u64* args,
                               u64 nargs, u64 ret_ty, bool var_args);

/* Search a matching function prototype for given operator and first argument
 * type ty_id
 * @node
 * @operator
 * @ty_id
 */
libexport ast_t* ast_search_fn_op(ast_t* node, int operator, u64 ty_id);

/* Search a matching function prototype given "expression call"
 * @id
 * @args_call
 */
libexport ast_t* ast_search_fn_wargs(string* id, ast_t* args_call);

/* Search all function with given name
 * @node
 * @id
 */
libexport array* ast_search_fns(ast_t* node, string* id);

/* Traverse given tree and return all aparences of given type
 * @node
 * @t
 * @return array or null
 */
libexport array* ast_search_node_type(ast_t* node, ast_types_t t);

/* cldoc:end-category() */
/*----------------------------------------------------------------------------*/
/* cldoc:begin-category(ast-reduce.c) */

/* Reduce some tree nodes, so the codegen isn't redundant
 *
 * @node should be root
 * @return node o error
 */
libexport ast_t* ast_reduce(ast_t* node);

/* cldoc:end-category() */

/* cldoc:begin-category(ast-implement.c) */

/* Implement a function template for a given call
 *
 * @node should be root
 * @return node o error
 */
libexport ast_t* ast_implement_fn(ast_t* call, ast_t* decl, string* uid);

/* Implement a function template for a given call
 *
 * @type_list types
 * @decl templated struct declaration
 * @uid unique id (0 auto-generated)
 * @return node
 */
libexport ast_t* ast_implement_struct(ast_t* type_list, ast_t* decl,
                                      string* uid);

/* cldoc:end-category() */

/* cldoc:begin-category(ast-modify.c) */

/* clear (memset 0) but keep parent and set type
 *
 * @node
 * @old type id
 * @new type id
 */
libexport void ast_clear(ast_t* node, ast_types_t type);

/* change all type found while traversing
 *
 * @node
 * @old type id
 * @new type id
 */
libexport void ast_replace_types(ast_t* node, u64 old, u64 new);

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
libexport void ast_reverse(ast_t* node, ast_cb_t cb, ast_t* parent, u64 level,
                           void* userdata_in, void* userdata_out);

/* cldoc:end-category() */

/* cldoc:begin-category(ast-static.c) */

/* Return if the node is really a literal only
 * TODO review if logic is duplicated by [ast_id_literal](#ast_id_literal)
 * @node
 * @id
 */
libexport bool ast_is_static(ast_t* node);

libexport bool ast_is_left_value(ast_t* node);

/* cldoc:end-category() */
/* cldoc:begin-category(ast-codegen.c) */

/* Determine if given node require to be loaded to access its value
 *
 * @node
 * @return true if load is required
 */
libexport bool ast_require_load(ast_t* node);

/* cldoc:end-category() */
/* cldoc:begin-category(ast-dump.c) */

/* Debug: Print to stderr a node as text
 *
 * @node
 */
libexport void ast_dump_one(ast_t* node);

/* Debug: Recursive print to stderr a node as text
 *
 * @node
 */
libexport void ast_dump(ast_t* node);
/* Debug: Recursive print to stderr a node as text
 * but ignoring log level
 *
 * @node
 */
libexport void ast_dump_s(ast_t* node);

/* Debug: Recursive print to stderr a node as text (do not follow imports)
 *
 * @node
 */
libexport void ast_mindump(ast_t* node);

/* Debug: Recursive print to stderr a node as text and code
 *
 * @node
 */
libexport void ast_fulldump(ast_t* node);

/* cldoc:end-category() */
/* cldoc:begin-category(ast-error.c) */

/* Print an error to stderr
 *
 * @node
 */
libexport bool ast_print_error(ast_t* node);

/* Print an error an trace to stderr, then exit
 *
 * @node
 * @message
 */
libexport void ast_print_error_at(ast_t* node, char* message);

/* Print an error an trace to stderr, then exit
 *
 * @node
 * @message
 */
libexport void ast_raise_error(ast_t* node, char* message, ...);

/* cldoc:end-category() */

/* cldoc:begin-category(ast-query.c) */

/* Return if the node is a literal
 * Note: lunary + number is a literal (-8)
 * Note: binop (number + number) is a literal (8 + 7)
 * Etc...
 * @node
 * @return if @node is a literal
 */
libexport bool ast_is_literal(ast_t* node);

/* Return if the type of node is a pointer (at first level!)
 * @node
 */
libexport bool ast_is_pointer(ast_t* node);

/* Return if given node has at least one parent with the given type
 * @node
 * @type
 */
libexport bool ast_has_parent(ast_t* node, ast_types_t type);

/* Get the string piece of code given a node
 * NOTE Does not always work, because some nodes are auto-generated
 *
 * @node
 * @return root node
 */
libexport string* ast_get_code(ast_t* node);

/* Get attribute with given its label
 *
 * @list list of attributes
 * @needle attribute label
 * @return node if found, 0 otherwise
 */
libexport ast_t* ast_get_attribute(ast_t* list, string* needle);

/* Reverse the tree a get the global scope
 *
 * @node
 * @return node if found, 0 otherwise
 */
libexport ast_t* ast_get_global_scope(ast_t* node);

/* Reverse the tree searching nearest scope
 *
 * @node
 * @return neasert scope
 */
libexport ast_t* ast_get_scope(ast_t* node);
/* Get all scopes, in order!
 *
 * @node
 * @return 0 if no scope found, an array with the list of scopes.
 */
libexport array* ast_get_scopes(ast_t* node);

/* Reverse the tree searching nearest function scope
 *
 * @node
 * @return neasert scope
 */
libexport ast_t* ast_get_function_scope(ast_t* node);

/* Get location as string
 *
 * @node
 * @return string location
 */
libexport string* ast_get_location(ast_t* node);

/* Return root node
 * A root node is the program or the module
 *
 * @node
 * @return root node
 */
libexport ast_t* ast_get_root(ast_t* node);

/* Get tyid given a node
 * TODO this should be removed, in pro of just `node->ty_id`
 * @node
 */
libexport u64 ast_get_typeid(ast_t* node);

/* Return if index of given property
 * @decl
 * @id
 */
libexport u64 ast_get_struct_prop_idx(ast_t* decl, string* id);

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
libexport void ast_traverse(ast_t* node, ast_cb_t cb, ast_t* parent, u64 level,
                            void* userdata_in, void* userdata_out);

/* Traverse a list calling cb on each node
 * @node
 * @cb
 * @parent
 * @level
 * @userdata_in
 * @userdata_out
*/
libexport void ast_traverse_list(ast_t* node, ast_cb_t cb, ast_t* until,
                                 u64 level, void* userdata_in,
                                 void* userdata_out);

/* cldoc:end-category() */

/* cldoc:begin-category(ast-scope.c) */

libexport ast_t* ast_scope_var(ast_t* node, string* identifier);

libexport array* ast_scope_fns(ast_t* node, string* id);

libexport ast_t* ast_scope_type(ast_t* node, string* id);

/* Get declaration for given identifier (var/fn) reversing scopes.
 * @node
 * @identifier
 */
libexport ast_t* ast_scope_decl(ast_t* node, string* identifier);

/* cldoc:end-category() */
