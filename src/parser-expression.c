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

FL_READER_IMPL(expression) {
  fl_ast_t* ast;

  FL_TRY_READ(expr_assignment);

  return 0;
}

FL_READER_IMPL(expr_assignment) {
  fl_ast_t* ast;

  FL_TRY_READ(expr_assignment_full);
  FL_TRY_READ(expr_conditional);

  return 0;
}

FL_READER_IMPL(expr_assignment_full) {
  FL_AST_START(FL_AST_EXPR_ASSIGNAMENT);

  ast->assignament.left = FL_READ(expr_lhs);
  // TODO manage errors
  if (!ast->assignament.left) {
    FL_RETURN_NOT_FOUND();
  }

  // now token must be an assignament operator

  switch (state->token->type) {
  case FL_TK_ASTERISKEQUAL:
  case FL_TK_SLASHEQUAL:
  case FL_TK_MINUSQUAL:
  case FL_TK_PLUSEQUAL:
  case FL_TK_MODEQUAL:
  case FL_TK_GT3EQUAL:
  case FL_TK_GT2EQUAL:
  case FL_TK_LT2EQUAL:
  case FL_TK_QMARKEQUAL:
  case FL_TK_TEQUAL:
  case FL_TK_EEQUAL:
  case FL_TK_CEQUAL:
  case FL_TK_OREQUAL:
  case FL_TK_ANDEQUAL:
  case FL_TK_ASSIGNAMENT:
    ast->assignament.operator= state->token->type;
  default:
    FL_RETURN_NOT_FOUND();
  }
  FL_NEXT();

  ast->assignament.right = FL_READ(expr_assignment);
  // TODO manage errors

  return ast;
}

FL_READER_IMPL(expr_lhs) { return 0; }

FL_READER_IMPL(expr_conditional) {
  // FL_AST_START(FL_AST_EXPR_CONDITIONAL);

  fl_ast_t* left = FL_READ(expr_logical_or);

  if (!left) {
    // FL_RETURN_NOT_FOUND();
    return 0;
  }

  return left;
}

// TODO support multiple tokens
fl_ast_t* fl_read_binop(FL_READER_HEADER, fl_tokens_t token,
                        fl_reader_cb_t next) {
  // TODO resizable
  fl_ast_t** leafs = malloc(sizeof(fl_ast_t*) * 10);
  size_t leafs_s = 0;

  bool err_left = false;
  bool err_op = false;

  do {
    FL_AST_START(FL_AST_EXPR_BINOP);
    // printf("try to read left!\n");

    ast->binop.left = 0;
    ast->binop.right = 0;

    // printf("state before literal %lu [%p]\n", state->current,
    // ast->binop.left);
    ast->binop.left = next(FL_READER_HEADER_SEND);
    // printf("state after literal %lu [%p]\n", state->current,
    // ast->binop.left);

    if (!ast->binop.left) {
      err_left = true;
      fl_ast_delete(ast);
    } else {
      // try to read the operator
      // printf("push and try to read operator!\n");

      leafs[leafs_s++] = ast;

      // printf("token: %u == %u\n", state->token->type, token);
      // printf("token: %lu\n", state->current);

      if (state->token->type == token) {
        // printf("** operator found!\n");
        FL_NEXT();
        // push
        ast->binop.operator= token;
      } else {
        err_op = true;
      }
    }
  } while (!err_left && !err_op);

  // printf("leafs_s = %ld", leafs_s);

  if (!leafs_s) {
    free(leafs);
    return 0;
  }

  if (leafs_s == 1) {
    fl_ast_t* tmp = leafs[0]->binop.left;
    free(leafs[0]);
    free(leafs);
    return tmp;
  }

  // unroll

  size_t i = 0;

  // last-left is previous right
  fl_ast_t* tmp = leafs[leafs_s - 1]->binop.left;
  fl_ast_t* tmp2; // last
  for (; i < leafs_s - 2; ++i) {
    leafs[i]->binop.right = leafs[i + 1];
  }
  tmp2 = leafs[leafs_s - 2];
  tmp2->binop.right = tmp;
  free(leafs[leafs_s - 1]); // no recursive, just remove last binop

  tmp = leafs[0];
  free(leafs);

  return tmp;
}

FL_READER_IMPL(expr_logical_or) {
  return fl_read_binop(FL_READER_HEADER_SEND, FL_TK_PLUS,
                       FL_READER_FN(literal));
}
