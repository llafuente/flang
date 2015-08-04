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
  PSR_START(fn_node, FL_AST_DECL_FUNCTION);

  if (PSR_ACCEPT_TOKEN(FL_TK_FFI_C)) {
    fn_node->func.ffi = true;
    PSR_SKIPWS();
  }

  if (!PSR_ACCEPT_TOKEN(FL_TK_FUNCTION)) {
    PSR_RET_KO(fn_node);
  }
  PSR_SKIPWS();

  PSR_READ_OR_DIE(id, lit_identifier, { fl_ast_delete(fn_node); },
                  "cannot parse function identifier");

  fn_node->func.id = id;

  PSR_SKIPWS();

  // params
  PSR_EXPECT_TOKEN(FL_TK_LPARENTHESIS, fn_node, {}, "expected '('");

  if (!PSR_ACCEPT_TOKEN(FL_TK_RPARENTHESIS)) {
    PSR_START_LIST(list);
    fn_node->func.params = list;

    do {
      PSR_SKIPWS();

      // varargs must be the latest argument
      if (PSR_ACCEPT_TOKEN(FL_TK_3DOT)) {
        fn_node->func.varargs = true;
        PSR_SKIPWS();

        break;
      }

      PSR_READ_OR_DIE(param, parameter, { fl_ast_delete(fn_node); }, 0);

      list->list.elements[list->list.count++] = param;

      PSR_SKIPWS();
    } while (PSR_ACCEPT_TOKEN(FL_TK_COMMA));
    fn_node->func.nparams = list->list.count;

    PSR_EXPECT_TOKEN(FL_TK_RPARENTHESIS, fn_node, {
      fl_ast_delete(fn_node->func.id);
      fl_ast_delete_list(list);
    }, "expected ')'");
  }

  PSR_SKIPWS();

  // try to read return type

  if (PSR_ACCEPT_TOKEN(FL_TK_COLON)) {
    PSR_SKIPWS();

    PSR_READ_OR_DIE(ret_type, type, { fl_ast_delete(fn_node); }, 0);

    fn_node->func.ret_type = ret_type;

    PSR_SKIPWS();
  } else {
    PSR_CREATE(ty, FL_AST_TYPE);
    ty->ty_id = 1;
    fn_node->func.ret_type = ty; // void
  }

  PSR_SKIPWS();

  // check if it's only a declaration without body
  if (PSR_TEST_TOKEN(FL_TK_SEMICOLON)) {
    PSR_RET_OK(fn_node);
  }

  PSR_READ_OR_DIE(body, block, { fl_ast_delete(fn_node); },
                  "expected block of code");

  fn_node->func.body = body;

  PSR_RET_OK(fn_node);
}

PSR_READ_IMPL(stmt_return) {
  PSR_START(ast, FL_AST_STMT_RETURN);

  if (!PSR_ACCEPT_TOKEN(FL_TK_RETURN)) {
    PSR_RET_KO(ast);
  }

  PSR_SKIPWS();

  fl_ast_t* argument = PSR_READ(expression);

  if (argument->type == FL_AST_ERROR) { // hard error error
    fl_ast_delete(ast);
    return argument;
  }

  ast->ret.argument = argument;

  PSR_RET_OK(ast);
}

PSR_READ_IMPL(parameter_typed) {
  PSR_START(ast, FL_AST_PARAMETER);

  fl_ast_t* type = PSR_READ(type);
  // soft error
  if (!type) {
    PSR_RET_KO(ast);
  }
  if (type->type == FL_AST_ERROR) {
    fl_ast_delete(type);
    PSR_RET_KO(ast);
  }

  PSR_SKIPWS();

  PSR_READ_OR_DIE(id, lit_identifier, { fl_ast_delete(ast); },
                  "expected identifier");

  // set type of the identifier
  ast->param.type = type;
  ast->param.id = id;
  id->ty_id = type->ty_id;

  PSR_RET_OK(ast);
}

PSR_READ_IMPL(parameter_notyped) {
  PSR_START(ast, FL_AST_PARAMETER);

  // hard error
  ast->param.id = PSR_READ(lit_identifier);
  if (!ast->param.id) {
    PSR_SET_SYNTAX_ERROR(ast, "expected identifier");
    return ast;
  }

  PSR_CREATE(type, FL_AST_TYPE);
  type->ty_id = 0;
  ast->param.type = type;

  PSR_RET_OK(ast);
}

PSR_READ_IMPL(parameter) {
  fl_ast_t* ast;

  FL_TRY_READ(parameter_typed);
  FL_TRY_READ(parameter_notyped);

  // TODO read default
  // TODO read assertions!

  return 0;
}
