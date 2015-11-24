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

#include "flang.h"

ast_t* ast_err_node = 0;
char* ast_err_buff = 0;

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

void ast_print_error_lines(const string* line, st_len_t pos,
                           const string* code) {
  if (pos >= MAX(0, ast_err_node->first_line - 3) &&
      pos <= ast_err_node->last_line + 2) {
    fprintf(stderr, "%6d | %s\n", pos + 1, line->value);
    if (pos == ast_err_node->first_line - 1) {
      fprintf(stderr, "%*s\x1B[32m^--%s\x1B[39m\n",
              (int)(9 + ast_err_node->first_column), " ",
              ast_err_node->err.message->value);
    }
  }
}

bool ast_print_error(ast_t* node) {
  assert(node->type == FL_AST_PROGRAM);
  ast_t* err = node->program.body;

  if (err->type == FL_AST_ERROR) {
    fprintf(stderr, "\n\n\x1B[31mError: %s\x1B[39m\n", err->err.message->value);
    fprintf(stderr, "File & Line: %s:%d:%d @ %d:%d\n", node->program.file,
            err->first_line, err->first_column, err->last_line,
            err->last_column);

    ast_err_node = err;

    // TODO add context do not use global var
    st_line_iterator(node->program.code, ast_print_error_lines);

    // ast_dump(node);
    return true;
  }

  return false;
}

void ast_raise_error(ast_t* node, char* message) {
  fprintf(stderr, "\n\n\x1B[31mError: %s\x1B[39m\n", message);

  if (!node) {
    __sanitizer_print_stack_trace();
    exit(6);
    return;
  }
  ast_t* root = node;
  while (root->type != FL_AST_PROGRAM) {
    root = root->parent;
  }

  ast_dump(node); // even node parent or root?!

  fprintf(stderr, "File & Line: %s:%d:%d @ %d:%d\n", root->program.file,
          node->first_line, node->first_column, node->last_line,
          node->last_column);

  ast_err_node = node;

  // TODO add context do not use global var
  st_line_iterator(node->program.code, ast_print_error_lines);

  __sanitizer_print_stack_trace();
  exit(6);
}
