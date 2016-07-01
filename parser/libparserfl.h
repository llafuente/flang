#include "parser/grammar/parser.h"

/* cldoc:begin-category(parser/libparserfl.c) */

/* Parse an utf8 string and return the root ast node.
 *
 * see: [fl_parse_file](#fl_parse_file)
 *
 * @str input string
 * @return root ast node
 */
libexport ast_t* fl_parse_utf8(char* str);

/* Parse an utf8 string and return the root ast node
 * with import core at the beginning. So you can later call
 * ast_load_imports and get the full ast.
 *
 * see: [fl_parse_main_file](#fl_parse_main_file)
 *
 * @str input string
 * @return root ast node
 */
libexport ast_t* fl_parse_main_utf8(char* str);

/* Return file contents parsed.
 *
 * see: [fl_parse_utf8](#fl_parse_utf8)
 *
 * @filename path to file
 * @return root ast node
 */
libexport ast_t* fl_parse_file(const char* filename);

/* Return file contents parsed with import core at the beginning.
 * So you can later call ast_load_imports and get the full ast.
 *
 * see: [fl_parse_main_utf8](#fl_parse_main_utf8)
 *
 * @filename path to file
 * @return root ast node
 */
libexport ast_t* fl_parse_main_file(const char* filename);

/* Help function, exported to easy test code.
 *
 * @filename path to file
 * @return file contents as string
 */
libexport string* fl_file_to_string(const char* filename);

/* cldoc:end-category() */
/*----------------------------------------------------------------------------*/
/* cldoc:begin-category(parser/ast/ast.c) */

/* Traverse the tree and set parent
 * @root
 */
libexport void ast_parent(ast_t* root);

/* Get tyid given a node
 * TODO this should be removed, in pro of just `node->ty_id`
 * @node
 */
libexport u64 ast_get_typeid(ast_t* node);

/* Return if the type of node is a pointer (at first level!)
 * @node
 */
libexport bool ast_is_pointer(ast_t* node);
/* Return if index of given property
 * @decl
 * @id
 */
libexport u64 ast_get_struct_prop_idx(ast_t* decl, string* id);

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
                                ast_t* body, ast_t* attibutes);
libexport void ast_mk_fn_decl_body(ast_t* fn, ast_t* body);
libexport ast_t* ast_mk_fn_param(ast_t* id, ast_t* type, ast_t* def);
libexport ast_t* ast_mk_binop(ast_t* left, int op, ast_t* right);
libexport ast_t* ast_mk_assignament(ast_t* left, int op, ast_t* right);
libexport ast_t* ast_mk_call_expr(ast_t* callee, ast_t* arguments);
libexport ast_t* ast_mk_template(ast_t* id, ast_t* block);
libexport ast_t* ast_mk_type_auto();
libexport ast_t* ast_mk_type_void();
libexport ast_t* ast_mk_type(string* id, ast_t* child);
libexport ast_t* ast_mk_type2(ast_t* id, ast_t* child);
libexport ast_t* ast_mk_comment(string* text);
libexport ast_t* ast_mk_lunary(ast_t* element, int operator);
libexport ast_t* ast_mk_runary(ast_t* element, int operator);
libexport ast_t* ast_mk_if(ast_t* test, ast_t* block, ast_t* alternate);
libexport ast_t* ast_mk_loop(ast_t* init, ast_t* pre_cond, ast_t* update,
                             ast_t* bloc, ast_t* post_cond);
libexport ast_t* ast_mk_struct_decl(ast_t* id, ast_t* fields);
libexport ast_t* ast_mk_struct_decl_field(ast_t* id, ast_t* type);
libexport ast_t* ast_mk_member(ast_t* left, ast_t* property, bool expression);
libexport ast_t* ast_mk_sizeof(ast_t* type);
libexport ast_t* ast_mk_cast(ast_t* type, ast_t* element);
libexport ast_t* ast_mk_import(ast_t* string_lit, bool foward);
libexport ast_t* ast_mk_log(ast_t* list);
libexport ast_t* ast_mk_attribute(ast_t* id, ast_t* value);

/* cldoc:end-category() */
/*----------------------------------------------------------------------------*/
/* cldoc:begin-category(parser/ast/ast-get.c) */

/* Get tyid from given AST Identifier
 *
 * @id Identifier node
 * @return type id
 */
libexport u64 ast_get_ident_typeid(ast_t* id);

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

/* cldoc:end-category() */
/*----------------------------------------------------------------------------*/
/* cldoc:begin-category(parser/ast/ast-traverse.c) */

/* Traverse the tree calling cb on each node
 * @node
 * @cb
 * @parent
 * @level
 * @userdata_in
 * @userdata_out
 */
libexport void ast_traverse(ast_t* node, ast_cb_t cb, ast_t* parent,
                            u64 level, void* userdata_in,
                            void* userdata_out);

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
/*----------------------------------------------------------------------------*/
/* cldoc:begin-category(ast-search.c) */

/* From give node reverse the tree searching given identifier
 * @node
 * @identifier
 */
libexport ast_t* ast_search_id_decl(ast_t* node, string* identifier);

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

/* Traverse given tree and return all aparences of t
 * @node
 * @t
 * @return array or null
 */
libexport array* ast_search_node_type(ast_t* node, ast_types_t t);

/* cldoc:end-category() */
/*----------------------------------------------------------------------------*/
