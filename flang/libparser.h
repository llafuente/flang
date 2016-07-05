#include "parser/grammar/parser.h"
#include "parser/grammar/tokens.h"

/* cldoc:begin-category(parser/libparserfl.c) */

/* Parse an utf8 string and return the root ast node.
 *
 * see: [psr_file](#psr_file)
 *
 * @str input string
 * @return root ast node
 */
libexport ast_t* psr_str_utf8(char* str);

/* Parse an utf8 string and return the root ast node
 * with import core at the beginning. So you can later call
 * psr_ast_imports and get the full ast.
 *
 * see: [psr_file_main](#psr_file_main)
 *
 * @str input string
 * @return root ast node
 */
libexport ast_t* psr_str_utf8_main(char* str);

/* Return file contents parsed.
 *
 * see: [psr_str_utf8](#psr_str_utf8)
 *
 * @filename path to file
 * @return root ast node
 */
libexport ast_t* psr_file(const char* filename);

/* Return file contents parsed with import core at the beginning.
 * So you can later call psr_ast_imports and get the full ast.
 *
 * see: [psr_str_utf8_main](#psr_str_utf8_main)
 *
 * @filename path to file
 * @return root ast node
 */
libexport ast_t* psr_file_main(const char* filename);

/* Help function, exported to easy test code.
 *
 * @filename path to file
 * @return file contents as string
 */
libexport string* psr_file_to_string(const char* filename);

/* Traverse the tree loading and appending all imports.
 * Imports cannot be double loaded so it safe.
 * @node
 */
libexport ast_t* psr_ast_imports(ast_t* node);

/* cldoc:end-category() */
