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
