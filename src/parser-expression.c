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

  printf("(parser) expr_assignment_full left");

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
    FL_RETURN_NOT_FOUND();
  }
  FL_NEXT();
  printf("(parser) expr_assignment_full operator");

  fl_parser_skipws(tokens, state);

  ast->assignament.right = FL_READ(expr_assignment);
  // TODO manage errors
  if (!ast->assignament.right) {
    FL_RETURN_NOT_FOUND();
  }

  printf("(parser) expr_assignment_full right");

  return ast;
}

FL_READER_IMPL(expr_lhs) {
  fl_ast_t* ast;

  FL_TRY_READ(literal);
  // TODO
  // expr_call,
  // expr_new

  return 0;
}

FL_READER_IMPL(expr_conditional) {
  // FL_AST_START(FL_AST_EXPR_CONDITIONAL);

  fl_ast_t* left = FL_READ(expr_logical_or);

  if (!left) {
    // FL_RETURN_NOT_FOUND();
    return 0;
  }

  return left;
}

fl_ast_t* fl_read_binop(FL_READER_HEADER, fl_tokens_t operators[], size_t n_ops,
                        fl_reader_cb_t next) {
  // TODO resizable
  fl_ast_t** leafs = malloc(sizeof(fl_ast_t*) * 10);
  size_t leafs_s = 0;

  bool err_left = false;
  bool err_op = false;
  bool op_found = true;
  size_t ops;

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

      op_found = false;
      for (ops = 0; ops < n_ops; ++ops) {
        if (state->token->type == operators[ops]) {
          FL_NEXT();
          // push
          ast->binop.operator= operators[ops];
          op_found = true;
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
  fl_tokens_t operators[] = {FL_TK_OR2};
  return fl_read_binop(FL_READER_HEADER_SEND, operators, 1,
                       FL_READER_FN(expr_logical_and));
}

FL_READER_IMPL(expr_logical_and) {
  fl_tokens_t operators[] = {FL_TK_AND2};
  return fl_read_binop(FL_READER_HEADER_SEND, operators, 1,
                       FL_READER_FN(expr_bitwise_or));
}

FL_READER_IMPL(expr_bitwise_or) {
  fl_tokens_t operators[] = {FL_TK_OR};
  return fl_read_binop(FL_READER_HEADER_SEND, operators, 1,
                       FL_READER_FN(expr_bitwise_xor));
}

FL_READER_IMPL(expr_bitwise_xor) {
  fl_tokens_t operators[] = {FL_TK_CARET};
  return fl_read_binop(FL_READER_HEADER_SEND, operators, 1,
                       FL_READER_FN(expr_bitwise_and));
}

FL_READER_IMPL(expr_bitwise_and) {
  fl_tokens_t operators[] = {FL_TK_AND};
  return fl_read_binop(FL_READER_HEADER_SEND, operators, 1,
                       FL_READER_FN(expr_equality));
}

FL_READER_IMPL(expr_equality) {
  fl_tokens_t operators[] = {FL_TK_EEQUAL, FL_TK_EQUAL2};
  return fl_read_binop(FL_READER_HEADER_SEND, operators, 2,
                       FL_READER_FN(expr_relational));
}

FL_READER_IMPL(expr_relational) {
  fl_tokens_t operators[] = {FL_TK_LTE, FL_TK_LT, FL_TK_GTE, FL_TK_GT};
  return fl_read_binop(FL_READER_HEADER_SEND, operators, 4,
                       FL_READER_FN(expr_shift));
}

FL_READER_IMPL(expr_shift) {
  fl_tokens_t operators[] = {FL_TK_LT2, FL_TK_GT2};
  return fl_read_binop(FL_READER_HEADER_SEND, operators, 2,
                       FL_READER_FN(expr_additive));
}

FL_READER_IMPL(expr_additive) {
  fl_tokens_t operators[] = {FL_TK_PLUS, FL_TK_MINUS};
  return fl_read_binop(FL_READER_HEADER_SEND, operators, 2,
                       FL_READER_FN(expr_multiplicative));
}

FL_READER_IMPL(expr_multiplicative) {
  fl_tokens_t operators[] = {FL_TK_ASTERISK, FL_TK_SLASH, FL_TK_MOD};
  return fl_read_binop(FL_READER_HEADER_SEND, operators, 3,
                       FL_READER_FN(expr_unary));
}

FL_READER_IMPL(expr_unary) {
  fl_ast_t* ast;

  FL_TRY_READ(expr_unary_left);

  FL_TRY_READ(expr_unary_right);

  // lhs not needed, will be sent up by expr_unary_right
  // FL_TRY_READ(expr_lhs);

  return 0;
}

FL_READER_IMPL(expr_unary_left) {
  FL_AST_START(FL_AST_EXPR_LUNARY);

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
    FL_NEXT();
    break;
  default:
    FL_RETURN_NOT_FOUND();
  };

  ast->lunary.element = FL_READ(expr_lhs);
  // TODO handle errors
  if (!ast->lunary.element) {
    FL_RETURN_NOT_FOUND();
  }

  return ast;
}
FL_READER_IMPL(expr_unary_right) {
  FL_AST_START(FL_AST_EXPR_RUNARY);

  ast->runary.element = FL_READ(expr_lhs);

  // TODO handle errors
  if (!ast->runary.element) {
    FL_RETURN_NOT_FOUND();
  }

  // read operator
  switch (state->token->type) {
  case FL_TK_PLUS2:
  case FL_TK_MINUS2:
  case FL_TK_QMARK:
    ast->runary.operator= state->token->type;
    FL_NEXT();
    break;
  default: {
    // lhs is valid, send up!
    fl_ast_t* tmp = ast->runary.element;
    free(ast);
    return tmp;
  }
  };

  return ast;
}
