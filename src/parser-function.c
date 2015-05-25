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

FL_READER_IMPL(decl_function) {
  FL_AST_START(FL_AST_DECL_FUNCTION);

  if (!FL_ACCEPT_TOKEN(FL_TK_FUNCTION)) {
    FL_RETURN_NOT_FOUND();
  }

  // hard-errors!

  fl_parser_skipws(tokens, state);

  ast->func.id = FL_READ(lit_identifier);
  if (!ast->func.id) {
    FL_PARSER_ERROR(ast, "cannot parse function identifier");
    return ast;
  }

  fl_parser_skipws(tokens, state);

  // params
  if (!FL_ACCEPT_TOKEN(FL_TK_LPARANTHESIS)) {
    FL_PARSER_ERROR(ast, "expected '('");
    return ast;
  }

  if (!FL_ACCEPT_TOKEN(FL_TK_RPARANTHESIS)) {
    fl_ast_t** list = calloc(100, sizeof(fl_ast_t*));
    size_t i = 0;
    do {
      fl_parser_skipws(tokens, state);

      list[i] = FL_READ(lit_identifier);

      fl_parser_skipws(tokens, state);

      ++i;
    } while (FL_ACCEPT_TOKEN(FL_TK_COMMA));
    ast->func.params = list;

    if (!FL_ACCEPT_TOKEN(FL_TK_RPARANTHESIS)) {
      fl_ast_delete(ast->func.id);
      fl_ast_delete_list(list);
      FL_PARSER_ERROR(ast, "expected ')'");
      return ast;
    }
  }

  fl_parser_skipws(tokens, state);

  ast->func.body = FL_READ(block);

  fl_parser_skipws(tokens, state);

  return ast;
}
