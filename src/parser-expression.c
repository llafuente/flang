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
  FL_TRY_READ(cast);

  return 0;
}

PSR_READ_IMPL(expr_assignment_full) {
  PSR_START(ast, FL_AST_EXPR_ASSIGNAMENT);

  log_silly("left");
  ast->assignament.left = PSR_READ(expr_lhs);

  // TODO manage errors
  if (!ast->assignament.left) {
    log_silly("ko");
    PSR_RET_KO(ast);
  }

  // printf("(parser) expr_assignment_full left");

  PSR_SKIPWS();

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
    log_silly("ko 2");
    PSR_RET_KO(ast);
  }
  PSR_NEXT();
  // printf("(parser) expr_assignment_full operator");

  PSR_SKIPWS();

  log_silly("right");
  ast->assignament.right = PSR_READ(expr_assignment);
  // TODO manage errors
  if (!ast->assignament.right) {
    log_silly("ko 3");
    PSR_RET_KO(ast);
  }

  log_silly("ok");

  PSR_RET_OK(ast);
}

PSR_READ_IMPL(expr_lhs) {
  fl_ast_t* ast;

  // printf("**expr_lhs %s", state->token->string->value);

  FL_TRY_READ(expr_call);
  FL_TRY_READ(expr_primary);
  // TODO
  // expr_new

  return 0;
}
// - Literal
// - "(" expression ")"
PSR_READ_IMPL(expr_primary) {

  // TODO lit_array, lit_object?
  PSR_READ_OK(literal, literal);
  if (literal) {
    log_verbose("return literal");
    if (literal->type == FL_AST_LIT_IDENTIFIER) {
      literal->identifier.resolve = true;
    }
    return literal;
  }

  if (!PSR_ACCEPT_TOKEN(FL_TK_LPARENTHESIS)) {
    log_silly("no group");
    return 0;
  }

  log_silly("parenthesis");

  fl_ast_t* inside = PSR_READ(expr_logical_or); // TODO expression
  PSR_RET_IF_ERROR(inside, {});

  PSR_EXPECT_TOKEN(FL_TK_RPARENTHESIS, inside, {}, "expected ')'");

  log_verbose("return expression inside parenthesis");

  PSR_RET_OK(inside);
}

// TODO DO IT!
PSR_READ_IMPL(expr_new_full) {
  // "new" WS callee:MemberExpression WS args:Arguments
  return 0;
}

/*
rest:(
 property:Expression _W_ "]" {
  return { property: property, computed: true };
}
/  property:IdentifierName {
  return { property: property, computed: false };
}
)*
*/

PSR_READ_IMPL(expr_member) {
  log_verbose("read expr_primary, expr_new_full");

  psr_read_t left[] = {PSR_READ_NAME(expr_primary),
                       PSR_READ_NAME(expr_new_full)};
  fl_ast_t* member_left = psr_read_list(left, 2, PSR_READ_HEADER_SEND);
  if (!member_left) {
    log_silly("invalid member.left");
    return 0;
  }

  fl_ast_t* last = member_left;

  PSR_SKIPWS();

  // test "." and "["
  if (!PSR_TEST_TOKEN(FL_TK_DOT) && !PSR_TEST_TOKEN(FL_TK_LBRACKET)) {
    PSR_RET_OK(last);
  }

  do {
    PSR_START_FROM(last, member, FL_AST_EXPR_MEMBER);
    member->member.left = last;
    last = member;

    if (PSR_ACCEPT_TOKEN(FL_TK_DOT)) {
      log_silly("dot accesss");
      PSR_SKIPWS();
      PSR_READ_OR_DIE(property, lit_identifier, {
        fl_ast_delete(last);
        fl_ast_delete(property);
      }, "expected expression");
      log_silly("dot accesss - ok");
      last->member.property = property;

      PSR_END(member);
    } else if (PSR_ACCEPT_TOKEN(FL_TK_LBRACKET)) {
      log_silly("open bracket access: '%s'", state->token->string->value);
      PSR_SKIPWS();
      log_silly("\n\n**********************************");
      PSR_READ_OR_DIE(property, expression, {
        fl_ast_delete(last);
        fl_ast_delete(property);
      }, "expected identifier");
      log_silly("\n\n**********************************");
      log_silly("close bracket access: '%s'", state->token->string->value);

      last->member.property = property;
      last->member.expression = true;

      PSR_EXPECT_TOKEN(FL_TK_RBRACKET, last, {}, "expected ']'");

      log_silly("member read");
      PSR_END(member);
    }
  } while (PSR_TEST_TOKEN(FL_TK_DOT) || PSR_TEST_TOKEN(FL_TK_LBRACKET));

  fl_ast_debug(last);

  PSR_RET_OK(last);
}

// TODO or: "new" WS callee:NewExpression
PSR_READ_IMPL(expr_new) { return PSR_READ(expr_member); }

// TODO do it, is a passthough right now
PSR_READ_IMPL(expr_conditional) {
  // PSR_START(ast, FL_AST_EXPR_CONDITIONAL);

  fl_ast_t* left = PSR_READ(expr_logical_or);

  return left ? left : 0;
}

fl_ast_t* PSR_READ_binop(PSR_READ_HEADER, fl_tokens_t operators[], size_t n_ops,
                         psr_read_t next) {
  // TODO resizable
  fl_ast_t** leafs = malloc(sizeof(fl_ast_t*) * 10);
  size_t leafs_s = 0;

  bool err_left = false;
  bool err_op = false;
  bool op_found = true;
  size_t ops;

  do {
    PSR_SKIPWS();

    PSR_START(ast, FL_AST_EXPR_BINOP);

    ast->binop.left = 0;
    ast->binop.right = 0;

    // printf("state before literal %lu [%p]", state->current,
    // ast->binop.left);
    ast->binop.left = next(PSR_READ_HEADER_SEND);
    // printf("state after literal %lu [%p]", state->current,
    // ast->binop.left);

    if (!ast->binop.left) {
      err_left = true;
      fl_ast_delete(ast);
    } else {
      // try to read the operator
      leafs[leafs_s++] = ast;
      PSR_SKIPWS();

      op_found = false;
      for (ops = 0; ops < n_ops; ++ops) {
        if (state->token->type == operators[ops]) {
          PSR_NEXT();
          // push
          ast->binop.operator= operators[ops];
          op_found = true;
          PSR_END(ast);
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
  PSR_START(ast, FL_AST_EXPR_LUNARY);

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
    PSR_RET_KO(ast);
  };

  ast->lunary.element = PSR_READ(expr_lhs);
  // TODO handle errors
  if (!ast->lunary.element) {
    PSR_RET_KO(ast);
  }

  PSR_RET_OK(ast);
}
PSR_READ_IMPL(expr_unary_right) {
  PSR_START(ast, FL_AST_EXPR_RUNARY);

  ast->runary.element = PSR_READ(expr_lhs);

  // TODO handle errors
  if (!ast->runary.element) {
    PSR_RET_KO(ast);
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

  PSR_RET_OK(ast);
}

PSR_READ_IMPL(expr_argument) { return PSR_READ(expression); }

PSR_READ_IMPL(expr_call) {
  PSR_START(ecall, FL_AST_EXPR_CALL);

  log_verbose("read callee '%s'", state->token->string->value);
  PSR_READ_OK(callee, expr_member)
  if (!callee) {
    PSR_RET_KO(ecall); // soft
  }

  // TODO if return member expression undo the skip?
  PSR_SKIPWS();

  // at least return member expression
  if (!PSR_ACCEPT_TOKEN(FL_TK_LPARENTHESIS)) {
    log_verbose("return callee, there is no '(' '%s'",
                state->token->string->value);
    fl_ast_delete(ecall);
    return callee;
  }

  // from now on, hard!
  ecall->call.callee = callee;

  PSR_SKIPWS();

  PSR_START_LIST(list);
  ecall->call.arguments = list;
  do {
    PSR_SKIPWS();

    PSR_READ_OR_DIE(argument, expr_argument, { fl_ast_delete(ecall); },
                    "expected function argument");

    list->list.elements[list->list.count++] = argument;

    PSR_SKIPWS();
  } while (PSR_ACCEPT_TOKEN(FL_TK_COMMA));
  ecall->call.narguments = list->list.count;

  PSR_EXPECT_TOKEN(FL_TK_RPARENTHESIS, callee, {}, "expected ')'");

  PSR_RET_OK(ecall);
}
