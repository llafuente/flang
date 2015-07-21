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

PSR_READ_IMPL(stmt_if) {
  if (!PSR_TEST_TOKEN(FL_TK_IF)) {
    return 0;
  }

  cg_print("(parser) if start!") PSR_START(stmt, FL_AST_STMT_IF);

  PSR_ACCEPT_TOKEN(FL_TK_IF);

  fl_ast_t* t = PSR_READ(expression);
  cg_print("(parser) expression");
  fl_ast_debug(t);

  if (!t) {
    PSR_RET_SYNTAX_ERROR(stmt, "expected an expression");
  }
  PSR_RET_IF_ERROR(t, { fl_ast_delete(stmt); });

  fl_ast_t* body = PSR_READ(block);
  cg_print("(parser) body");
  PSR_RET_IF_ERROR(body, {
    fl_ast_delete(t);
    fl_ast_delete(stmt);
  });

  stmt->if_stmt.test = t;
  stmt->if_stmt.block = body;
  cg_print("(parser) if ok!");
  PSR_RET_OK(stmt);
}
