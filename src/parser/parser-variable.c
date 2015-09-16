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
// TODO declaration - declarator list
PSR_READ_IMPL(decl_variable) {
  ast_t* ast;
  FL_TRY_READ(decl_variable_with_type);
  FL_TRY_READ(decl_variable_no_type);

  return 0;
}

PSR_READ_IMPL(decl_variable_no_type) {
  PSR_START(ast, FL_AST_DTOR_VAR);

  tk_tokens_t tks[] = {FL_TK_VAR, FL_TK_UNVAR, FL_TK_CONST, FL_TK_STATIC,
                       FL_TK_GLOBAL};
  if (!psr_accept_token_list(tokens, state, tks, 5)) {
    PSR_RET_KO(ast);
  }

  PSR_SKIPWS();

  ast->var.id = PSR_READ(lit_identifier);
  if (!ast->var.id) {
    PSR_RET_KO(ast);
  }

  // this variable need to be inferred
  PSR_CREATE(type, FL_AST_TYPE);
  type->ty_id = 0;
  ast->var.type = type;

  PSR_RET_OK(ast);
}

PSR_READ_IMPL(decl_variable_with_type) {
  log_silly("has var, unvar, const, static, global?");
  PSR_START(ast, FL_AST_DTOR_VAR);

  tk_tokens_t tks[] = {FL_TK_VAR, FL_TK_UNVAR, FL_TK_CONST, FL_TK_STATIC,
                       FL_TK_GLOBAL};
  if (!psr_accept_token_list(tokens, state, tks, 5)) {
    PSR_RET_KO(ast);
  }

  PSR_SKIPWS();
  PSR_READ_OK(type, type);

  if (!type) {
    log_silly("no type...");
    PSR_RET_KO(ast);
  }

  ast->var.type = type;

  PSR_SKIPWS();

  // TODO READ_OR_DIE!
  PSR_READ_OK(id, lit_identifier)

  if (!id) {
    log_silly("no id?");
    PSR_RET_KO(ast);
  }
  ast->var.id = id;

  log_verbose("type decl [%zu] '%s'\n", ast->var.type->ty_id,
              id->identifier.string->value);

  ast->var.id->ty_id = ast->var.type->ty_id;
  ast->ty_id = ast->var.type->ty_id;

  PSR_RET_OK(ast);
}
