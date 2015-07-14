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

PSR_READ_IMPL(decl_function) {
  PSR_AST_START(FL_AST_DECL_FUNCTION);

  if (PSR_ACCEPT_TOKEN(FL_TK_FFI_C)) {
    ast->func.ffi = true;
    fl_parser_skipws(tokens, state);
  }

  if (!PSR_ACCEPT_TOKEN(FL_TK_FUNCTION)) {
    PSR_AST_RET_NULL();
  }

  // hard-errors!

  fl_parser_skipws(tokens, state);

  ast->func.id = PSR_READ(lit_identifier);
  if (!ast->func.id) {
    PSR_SYNTAX_ERROR(ast, "cannot parse function identifier");
    return ast;
  }

  fl_parser_skipws(tokens, state);

  // params
  if (!PSR_ACCEPT_TOKEN(FL_TK_LPARENTHESIS)) {
    PSR_SYNTAX_ERROR(ast, "expected '('");
    return ast;
  }

  if (!PSR_ACCEPT_TOKEN(FL_TK_RPARENTHESIS)) {
    fl_ast_t** list = calloc(100, sizeof(fl_ast_t*)); // TODO resize support
    ast->func.params = list;

    size_t i = 0;
    do {
      fl_parser_skipws(tokens, state);

      // varargs must be the latest argument
      if (PSR_ACCEPT_TOKEN(FL_TK_3DOT)) {
        ast->func.varargs = true;
        fl_parser_skipws(tokens, state);

        break;
      }

      fl_ast_t* param = PSR_READ(parameter);

      if (param->type == FL_AST_ERROR) { // hard error error
        fl_ast_delete(ast);
        return param;
      }

      list[i++] = param;

      fl_parser_skipws(tokens, state);
    } while (PSR_ACCEPT_TOKEN(FL_TK_COMMA));
    ast->func.nparams = i;

    if (!PSR_ACCEPT_TOKEN(FL_TK_RPARENTHESIS)) {
      fl_ast_delete(ast->func.id);
      fl_ast_delete_list(list);

      PSR_SYNTAX_ERROR(ast, "expected ')'");
      return ast;
    }
  }

  fl_parser_skipws(tokens, state);

  // try to read return type

  if (PSR_ACCEPT_TOKEN(FL_TK_COLON)) {
    fl_parser_skipws(tokens, state);

    fl_ast_t* rtype = PSR_READ(type);
    if (rtype->type == FL_AST_ERROR) { // hard error error
      fl_ast_delete(ast);
      return rtype;
    }
    ast->func.ret_type = rtype;

    fl_parser_skipws(tokens, state);
  } else {
    PSR_AST_DUMMY(ty, FL_AST_TYPE);
    ty->ty.id = 1;
    ast->func.ret_type = ty; // void
  }

  fl_parser_skipws(tokens, state);

  // check if it's only a declaration without body
  if (PSR_TEST_TOKEN(FL_TK_SEMICOLON)) {
    PSR_AST_RET();
  }

  // block always return
  fl_ast_t* body = PSR_READ(block);

  if (body->type == FL_AST_ERROR) { // hard error error
    fl_ast_delete(ast);
    return body;
  }

  ast->func.body = body;

  PSR_AST_RET();
}

PSR_READ_IMPL(stmt_return) {
  PSR_AST_START(FL_AST_STMT_RETURN);

  if (!PSR_ACCEPT_TOKEN(FL_TK_RETURN)) {
    PSR_AST_RET_NULL();
  }

  fl_parser_skipws(tokens, state);

  fl_ast_t* argument = PSR_READ(expression);

  if (argument->type == FL_AST_ERROR) { // hard error error
    fl_ast_delete(ast);
    return argument;
  }

  ast->ret.argument = argument;

  PSR_AST_RET();
}

PSR_READ_IMPL(parameter_typed) {
  PSR_AST_START(FL_AST_PARAMETER);

  ast->param.type = PSR_READ(type);

  // soft error
  if (!ast->param.type) {
    PSR_AST_RET_NULL();
  }

  fl_parser_skipws(tokens, state);

  // hard error
  ast->param.id = PSR_READ(lit_identifier);
  if (!ast->param.id) {
    PSR_SYNTAX_ERROR(ast, "expected identifier");
  }

  PSR_AST_RET();
}

PSR_READ_IMPL(parameter_notyped) {
  PSR_AST_START(FL_AST_PARAMETER);

  // hard error
  ast->param.id = PSR_READ(lit_identifier);
  if (!ast->param.id) {
    PSR_SYNTAX_ERROR(ast, "expected identifier");
    return ast;
  }

  PSR_AST_DUMMY(type, FL_AST_TYPE);
  type->ty.id = 0;
  ast->param.type = type;

  PSR_AST_RET();
}

PSR_READ_IMPL(parameter) {
  fl_ast_t* ast;

  FL_TRY_READ(parameter_typed);
  FL_TRY_READ(parameter_notyped);

  // TODO read default
  // TODO read assertions!

  return 0;
}
