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

PSR_READ_IMPL(expression) {
  fl_ast_t* ast;

  FL_TRY_READ(expr_assignment);

  return 0;
}

PSR_READ_IMPL(expr_assignment) {
  fl_ast_t* ast;

  FL_TRY_READ(expr_assignment_full);
  FL_TRY_READ(expr_conditional);

  return 0;
}

PSR_READ_IMPL(expr_assignment_full) {
  PSR_AST_START(FL_AST_EXPR_ASSIGNAMENT);

  ast->assignament.left = PSR_READ(expr_lhs);

  // TODO manage errors
  if (!ast->assignament.left) {
    PSR_AST_RET_NULL();
  }

  //printf("(parser) expr_assignment_full left\n");

  fl_parser_skipws(tokens, state);

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
  case FL_TK_EQUAL:
    ast->assignament.operator= state->token->type;
    break;
  default:
    PSR_AST_RET_NULL();
  }
  PSR_NEXT();
  //printf("(parser) expr_assignment_full operator\n");

  fl_parser_skipws(tokens, state);

  ast->assignament.right = PSR_READ(expr_assignment);
  // TODO manage errors
  if (!ast->assignament.right) {
    PSR_AST_RET_NULL();
  }

  //printf("(parser) expr_assignment_full right\n");

  PSR_AST_RET();
}

PSR_READ_IMPL(expr_lhs) {
  fl_ast_t* ast;

  FL_TRY_READ(literal);
  // TODO
  // expr_call,
  // expr_new

  return 0;
}

PSR_READ_IMPL(expr_conditional) {
  // PSR_AST_START(FL_AST_EXPR_CONDITIONAL);

  fl_ast_t* left = PSR_READ(expr_logical_or);

  if (!left) {
    // PSR_AST_RET_NULL();
    return 0;
  }

  return left;
}

fl_ast_t* PSR_READ_binop(PSR_READ_HEADER, fl_tokens_t operators[], size_t n_ops,
                         fl_read_cb_t next) {
  // TODO resizable
  fl_ast_t** leafs = malloc(sizeof(fl_ast_t*) * 10);
  size_t leafs_s = 0;

  bool err_left = false;
  bool err_op = false;
  bool op_found = true;
  size_t ops;

  do {
    fl_parser_skipws(tokens, state);

    PSR_AST_START(FL_AST_EXPR_BINOP);
    // printf("try to read left!\n");

    ast->binop.left = 0;
    ast->binop.right = 0;

    // printf("state before literal %lu [%p]\n", state->current,
    // ast->binop.left);
    ast->binop.left = next(PSR_READ_HEADER_SEND);
    // printf("state after literal %lu [%p]\n", state->current,
    // ast->binop.left);

    if (!ast->binop.left) {
      err_left = true;
      fl_ast_delete(ast);
    } else {
      // try to read the operator
      // printf("push and try to read operator!\n");

      leafs[leafs_s++] = ast;
      fl_parser_skipws(tokens, state);

      // printf("token: %u == %u\n", state->token->type, token);
      // printf("token: %lu\n", state->current);

      op_found = false;
      for (ops = 0; ops < n_ops; ++ops) {
        if (state->token->type == operators[ops]) {
          PSR_NEXT();
          // push
          ast->binop.operator= operators[ops];
          op_found = true;
          PSR_AST_END();
          break;
        }
      }

      if (!op_found) {
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
  // TODO handle token_end properly while unrolling!

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

PSR_READ_IMPL(expr_logical_or) {
  fl_tokens_t operators[] = {FL_TK_OR2};
  return PSR_READ_binop(PSR_READ_HEADER_SEND, operators, 1,
                        PSR_READ_NAME(expr_logical_and));
}

PSR_READ_IMPL(expr_logical_and) {
  fl_tokens_t operators[] = {FL_TK_AND2};
  return PSR_READ_binop(PSR_READ_HEADER_SEND, operators, 1,
                        PSR_READ_NAME(expr_bitwise_or));
}

PSR_READ_IMPL(expr_bitwise_or) {
  fl_tokens_t operators[] = {FL_TK_OR};
  return PSR_READ_binop(PSR_READ_HEADER_SEND, operators, 1,
                        PSR_READ_NAME(expr_bitwise_xor));
}

PSR_READ_IMPL(expr_bitwise_xor) {
  fl_tokens_t operators[] = {FL_TK_CARET};
  return PSR_READ_binop(PSR_READ_HEADER_SEND, operators, 1,
                        PSR_READ_NAME(expr_bitwise_and));
}

PSR_READ_IMPL(expr_bitwise_and) {
  fl_tokens_t operators[] = {FL_TK_AND};
  return PSR_READ_binop(PSR_READ_HEADER_SEND, operators, 1,
                        PSR_READ_NAME(expr_equality));
}

PSR_READ_IMPL(expr_equality) {
  fl_tokens_t operators[] = {FL_TK_EEQUAL, FL_TK_EQUAL2};
  return PSR_READ_binop(PSR_READ_HEADER_SEND, operators, 2,
                        PSR_READ_NAME(expr_relational));
}

PSR_READ_IMPL(expr_relational) {
  fl_tokens_t operators[] = {FL_TK_LTE, FL_TK_LT, FL_TK_GTE, FL_TK_GT};
  return PSR_READ_binop(PSR_READ_HEADER_SEND, operators, 4,
                        PSR_READ_NAME(expr_shift));
}

PSR_READ_IMPL(expr_shift) {
  fl_tokens_t operators[] = {FL_TK_LT2, FL_TK_GT2};
  return PSR_READ_binop(PSR_READ_HEADER_SEND, operators, 2,
                        PSR_READ_NAME(expr_additive));
}

PSR_READ_IMPL(expr_additive) {
  fl_tokens_t operators[] = {FL_TK_PLUS, FL_TK_MINUS};
  return PSR_READ_binop(PSR_READ_HEADER_SEND, operators, 2,
                        PSR_READ_NAME(expr_multiplicative));
}

PSR_READ_IMPL(expr_multiplicative) {
  fl_tokens_t operators[] = {FL_TK_ASTERISK, FL_TK_SLASH, FL_TK_MOD};
  return PSR_READ_binop(PSR_READ_HEADER_SEND, operators, 3,
                        PSR_READ_NAME(expr_unary));
}

PSR_READ_IMPL(expr_unary) {
  fl_ast_t* ast;

  FL_TRY_READ(expr_unary_left);

  FL_TRY_READ(expr_unary_right);

  // lhs not needed, will be sent up by expr_unary_right
  // FL_TRY_READ(expr_lhs);

  return 0;
}

PSR_READ_IMPL(expr_unary_left) {
  PSR_AST_START(FL_AST_EXPR_LUNARY);

  // read operator
  switch (state->token->type) {
  case FL_TK_PLUS2:
  case FL_TK_MINUS2:
  case FL_TK_PLUS:
  case FL_TK_MINUS:
  case FL_TK_TILDE:
  case FL_TK_EXCLAMATION:
  case FL_TK_DELETE:
    // case FL_TK_ASTERISK: // dereference pointer
    // case FL_TK_TYPEOF:
    // case FL_TK_CLONE:
    ast->lunary.operator= state->token->type;
    PSR_NEXT();
    break;
  default:
    PSR_AST_RET_NULL();
  };

  ast->lunary.element = PSR_READ(expr_lhs);
  // TODO handle errors
  if (!ast->lunary.element) {
    PSR_AST_RET_NULL();
  }

  PSR_AST_RET();
}
PSR_READ_IMPL(expr_unary_right) {
  PSR_AST_START(FL_AST_EXPR_RUNARY);

  ast->runary.element = PSR_READ(expr_lhs);

  // TODO handle errors
  if (!ast->runary.element) {
    PSR_AST_RET_NULL();
  }

  // read operator
  switch (state->token->type) {
  case FL_TK_PLUS2:
  case FL_TK_MINUS2:
  case FL_TK_QMARK:
    ast->runary.operator= state->token->type;
    PSR_NEXT();
    break;
  default: {
    // lhs is valid, send up!
    fl_ast_t* tmp = ast->runary.element;
    free(ast);
    return tmp;
  }
  };

  PSR_AST_RET();
}
