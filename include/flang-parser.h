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

//-
//- MACROS
//-

//- types

struct psr_state {
  size_t current;
  tk_token_t* token;
  tk_token_t* prev_token;
  tk_token_t* next_token;
  size_t look_ahead_idx;
};

// TODO resize support
struct psr_stack {
  size_t current;
  fl_psrstate_t states[500];
};

//- declaration, parameter, calling, arguments...

#define PSR_READ_NAME(name) psr_##name

#define PSR_READ_DECL(name)                                                    \
  FL_EXTERN ast_t* PSR_READ_NAME(name)(PSR_READ_HEADER)

#define PSR_READ_HEADER                                                        \
  tk_token_list_t* tokens, fl_psrstack_t* stack, fl_psrstate_t* state

#define PSR_READ_HEADER_SEND tokens, stack, state

#define PSR_READ_IMPL(name) ast_t* PSR_READ_NAME(name)(PSR_READ_HEADER)

// printf(#name "\n");
#define PSR_READ(name) PSR_READ_NAME(name)(tokens, stack, state);

//- parser implementation helpers

//- accept
#define PSR_ACCEPT(string) psr_accept(tokens, state, string)

#define PSR_TEST_TOKEN(token_type) (state->token->type == token_type)

#define PSR_ACCEPT_TOKEN(token_type) psr_accept_token(tokens, state, token_type)

//- expect

#define PSR_EXPECT_TOKEN(token_type, target, err_block, err_str)               \
  if (!PSR_ACCEPT_TOKEN(token_type)) {                                         \
    err_block;                                                                 \
    ast_delete_props(target);                                                  \
    PSR_SET_SYNTAX_ERROR(target, err_str);                                     \
    target->token_start = state->token;                                        \
    return target;                                                             \
  }

//- move

#define PSR_NEXT() psr_next(tokens, state)
#define PSR_SKIPWS() psr_skipws(tokens, state)

//- ast

#define PSR_CREATE(target, ast_type)                                           \
  ast_t* target = (ast_t*)calloc(1, sizeof(ast_t));                            \
  target->type = ast_type;

#define PSR_START(target, ast_type)                                            \
  PSR_CREATE(target, ast_type)                                                 \
  target->token_start = state->token;

// use src node as start point
#define PSR_START_FROM(src, target, ast_type)                                  \
  PSR_CREATE(target, ast_type)                                                 \
  target->token_start = src->token_start;

#define PSR_START_LIST(target)                                                 \
  PSR_START(target, FL_AST_LIST);                                              \
  target->list.count = 0;                                                      \
  target->list.elements = calloc(100, sizeof(ast_t*));

#define PSR_END(target)                                                        \
  if (target->type != FL_AST_ERROR) {                                          \
    target->token_end = state->token;                                          \
  }

#define PSR_EXTEND(target, name)                                               \
  PSR_READ_NAME(name)(tokens, stack, state, &target)

// read that can raise errors but 'dont throw'
#define PSR_SOFT_READ(target, name)                                            \
  psr_look_ahead(stack, state);                                                \
  target = PSR_READ(name);                                                     \
  if (target) {                                                                \
    /*has errors?*/                                                            \
    if (target->type != FL_AST_ERROR) {                                        \
      psr_commit(stack, state);                                                \
      return target;                                                           \
    }                                                                          \
    ast_delete(target);                                                        \
    target = 0;                                                                \
  }                                                                            \
  psr_rollback(stack, state);

#define PSR_MUST_READ(name, target, block_err, err_ast, err_str)               \
  psr_look_ahead(stack, state);                                                \
  ast_t* target = PSR_READ_NAME(name)(tokens, stack, state);                   \
  if (!target) {                                                               \
    psr_rollback(stack, state);                                                \
    block_err PSR_RET_SYNTAX_ERROR(err_ast, err_str);                          \
  }                                                                            \
  if (target->type != FL_AST_ERROR) {                                          \
    psr_rollback(stack, state);                                                \
    block_err return target;                                                   \
  }

// read that can raise errors but 'dont throw'
#define PSR_RET_READED(target, name)                                           \
  psr_look_ahead(stack, state);                                                \
  target = PSR_READ(name);                                                     \
  if (target) {                                                                \
    /*has errors?*/                                                            \
    psr_commit(stack, state);                                                  \
    return target;                                                             \
  }                                                                            \
  psr_rollback(stack, state);

#define PSR_RET_IF_ERROR(target, block)                                        \
  if (target->type == FL_AST_ERROR) {                                          \
    block return target;                                                       \
  }

#define PSR_RET_IF_ERROR_OR_NULL(target, block)                                \
  if (!target || target->type == FL_AST_ERROR) {                               \
    block return target;                                                       \
  }

#define PSR_RET_OK(target)                                                     \
  PSR_END(target);                                                             \
  return target;

#define PSR_RET_KO(target)                                                     \
  if (target) {                                                                \
    ast_delete(target);                                                        \
  }                                                                            \
  return 0;

// target allow to reuse current ast
#define PSR_SET_SYNTAX_ERROR(target, string)                                   \
  target->type = FL_AST_ERROR;                                                 \
  target->token_end = state->token;                                            \
  target->err.str = string;                                                    \
  target->err.zone = FL_ERROR_SYNTAX;

#define PSR_RET_SYNTAX_ERROR(target, string)                                   \
  log_verbose("(psr-err) %s\n", string);                                       \
  PSR_SET_SYNTAX_ERROR(target, string)                                         \
  return target;

// printf("%s %p type %d\n", __FUNCTION__, ast, ast->type);
// TODO handle errors when done :)
#define FL_TRY_READ(name)                                                      \
  psr_look_ahead(stack, state);                                                \
  ast = PSR_READ(name);                                                        \
  if (ast) {                                                                   \
    /*handle errors*/                                                          \
    if (ast->type == FL_AST_ERROR) {                                           \
      psr_rollback(stack, state);                                              \
      return ast;                                                              \
    }                                                                          \
    psr_commit(stack, state);                                                  \
    return ast;                                                                \
  }                                                                            \
  psr_rollback(stack, state);

#define PSR_READ_OR_DIE(target, name, err_block, syntax_err)                   \
  psr_look_ahead(stack, state);                                                \
  ast_t* target = PSR_READ(name);                                              \
  if (!target) {                                                               \
    if (syntax_err) {                                                          \
      PSR_START(err_node, FL_AST_ERROR);                                       \
      psr_rollback(stack, state);                                              \
      err_block;                                                               \
      PSR_RET_SYNTAX_ERROR(err_node, (char*)syntax_err);                       \
    }                                                                          \
    psr_rollback(stack, state);                                                \
    err_block;                                                                 \
    return 0;                                                                  \
  } else if (target->type == FL_AST_ERROR) {                                   \
    psr_rollback(stack, state);                                                \
    err_block;                                                                 \
    return target;                                                             \
  } else {                                                                     \
    psr_commit(stack, state);                                                  \
  }

// ignore error (target = 0)
#define PSR_READ_OK(target, name)                                              \
  psr_look_ahead(stack, state);                                                \
  ast_t* target = PSR_READ(name);                                              \
  if (!target) {                                                               \
    psr_rollback(stack, state);                                                \
  } else if (target->type == FL_AST_ERROR) {                                   \
    psr_rollback(stack, state);                                                \
    ast_delete(target);                                                        \
    target = 0;                                                                \
  } else {                                                                     \
    psr_commit(stack, state);                                                  \
  }
