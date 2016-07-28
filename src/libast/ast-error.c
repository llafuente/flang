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

#include "flang/flang.h"
#include "flang/libast.h"
#include "flang/debug.h"
#include <setjmp.h>

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

void __ast_print_error_lines(const string* line, st_len_t pos,
                             const string* code) {
  if (pos >= MAX(0, ast_last_error_node->first_line - 3) &&
      pos <= ast_last_error_node->last_line + 2) {
    fprintf(stderr, "%6d | %s\n", pos + 1, line->value);
    if (pos == ast_last_error_node->first_line - 1) {
      fprintf(stderr, "%*s\x1B[32m^-- %s\x1B[39m\n",
              (int)(8 + ast_last_error_node->first_column), " ",
              ast_last_error_message);
    }
  }
}

bool ast_print_error(ast_t* node) {
  if (ast_last_error_message)
    return true;

  assert(node->type == AST_PROGRAM);
  ast_t* err = node->program.body;

  if (err->type == AST_ERROR) {
    fprintf(stderr, "\n\n\x1B[31mError: %s\x1B[39m\n", err->err.message->value);
    fprintf(stderr, "File & Line: %s:%d:%d @ %d:%d\n\n",
            node->program.file->value, err->first_line, err->first_column,
            err->last_line, err->last_column);

    ast_last_error_node = err;
    ast_last_error_message = ast_last_error_node->err.message->value;

    // TODO add context do not use global var
    st_line_iterator(node->program.code, __ast_print_error_lines);
    fprintf(stderr, "\n");

    // ast_dump(node);
    return true;
  }

  return false;
}

void ast_print_error_at(ast_t* node, char* message) {
  int bk = log_debug_level;
  log_debug_level = 99;

  ast_t* root = ast_get_root(node);
  ast_dump(node); // even node parent or root?!

  // search a decent node to display 'error area'
  ast_last_error_node = node;
  ast_last_error_message = message;

  if (!node->first_line && !node->first_column && !node->last_line &&
      !node->last_column) {
    do {
      node = node->parent;
    } while (!node->first_line && !node->first_column && !node->last_line &&
             !node->last_column);
    ast_last_error_node->first_line = node->first_line;
    ast_last_error_node->first_column = node->first_column;
    ast_last_error_node->last_line = node->last_line;
    ast_last_error_node->last_column = node->last_column;
  }

  fprintf(stderr, "File & Line: %s:%d:%d @ %d:%d\n\n",
          root->program.file->value, node->first_line, node->first_column,
          node->last_line, node->last_column);

  // TODO add context do not use global var
  st_line_iterator(root->program.code, __ast_print_error_lines);

  log_debug_level = bk;
}

void ast_raise_error(ast_t* node, char* message, ...) {
  // just once!
  if (ast_last_error_message)
    return;

  char* buffer = (char*)pool_new(1024);
  va_list args;
  va_start(args, message);
  vsprintf(buffer, message, args);
  va_end(args);

  fprintf(stderr, "\n\n\x1B[31mError: %s\x1B[39m\n", buffer);

  if (!node) {
    __sanitizer_print_stack_trace();
    return;
  }

  ast_print_error_at(node, buffer);

  fprintf(stderr, "\n\nStackTrace:\n");

  __sanitizer_print_stack_trace();

  longjmp(fl_on_error_jmp, 0);
}
