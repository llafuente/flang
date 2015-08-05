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

PSR_READ_IMPL(stmt_for) {
  if (!PSR_TEST_TOKEN(FL_TK_FOR)) {
    return 0;
  }

  log_debug("(parser) if start!");
  PSR_START(stmt, FL_AST_STMT_LOOP);
  stmt->loop.type = FL_AST_STMT_FOR;

  PSR_ACCEPT_TOKEN(FL_TK_FOR);
  PSR_SKIPWS();

  PSR_READ_OR_DIE(init, expression, { fl_ast_delete(stmt); },
                  "expected initialization expression");

  stmt->loop.init = init;

  PSR_SKIPWS();
  PSR_EXPECT_TOKEN(FL_TK_SEMICOLON, stmt, {}, "expected semicolon");
  PSR_SKIPWS();

  PSR_READ_OR_DIE(pre_cond, expression, { fl_ast_delete(stmt); },
                  "expected condition expression");

  stmt->loop.pre_cond = pre_cond;

  PSR_SKIPWS();
  PSR_EXPECT_TOKEN(FL_TK_SEMICOLON, stmt, {}, "expected semicolon");
  PSR_SKIPWS();

  PSR_READ_OR_DIE(update, expression, { fl_ast_delete(stmt); },
                  "expected update expression");

  stmt->loop.update = update;

  PSR_SKIPWS();

  PSR_READ_OR_DIE(block, block, { fl_ast_delete(stmt); },
                  "expected block of code");

  stmt->loop.block = block;

  PSR_RET_OK(stmt);
}

PSR_READ_IMPL(stmt_while) {
  if (!PSR_TEST_TOKEN(FL_TK_WHILE)) {
    return 0;
  }

  log_debug("(parser) if start!");
  PSR_START(stmt, FL_AST_STMT_LOOP);
  stmt->loop.type = FL_AST_STMT_WHILE;

  PSR_ACCEPT_TOKEN(FL_TK_WHILE);
  PSR_SKIPWS();

  PSR_READ_OR_DIE(pre_cond, expression, { fl_ast_delete(stmt); },
                  "expected condition expression");

  stmt->loop.pre_cond = pre_cond;

  PSR_READ_OR_DIE(block, block, { fl_ast_delete(stmt); },
                  "expected block of code");

  stmt->loop.block = block;

  PSR_RET_OK(stmt);
}

PSR_READ_IMPL(stmt_dowhile) {
  if (!PSR_TEST_TOKEN(FL_TK_DO)) {
    return 0;
  }

  log_debug("(parser) if start!");
  PSR_START(stmt, FL_AST_STMT_LOOP);
  stmt->loop.type = FL_AST_STMT_DOWHILE;

  PSR_ACCEPT_TOKEN(FL_TK_DO);
  PSR_SKIPWS();

  PSR_READ_OR_DIE(block, block, { fl_ast_delete(stmt); },
                  "expected block of code");

  stmt->loop.block = block;

  PSR_SKIPWS();
  PSR_EXPECT_TOKEN(FL_TK_WHILE, stmt, {}, "expected 'while'");
  PSR_SKIPWS();

  PSR_READ_OR_DIE(post_cond, expression, { fl_ast_delete(stmt); },
                  "expected condition expression");

  stmt->loop.post_cond = post_cond;

  PSR_RET_OK(stmt);
}
