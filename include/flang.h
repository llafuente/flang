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

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>

#include "stringc.h"

//-
//- types
//-

enum fl_tokens {
  FL_TK_UNKOWN,
  FL_TK_EOF,
  FL_TK_NEWLINE,
  FL_TK_LOG,
  FL_TK_FUNCTION,
  FL_TK_VAR,
  FL_TK_UNVAR,
  FL_TK_CONST,
  FL_TK_STATIC,
  FL_TK_GLOBAL,
  FL_TK_LPARANTHESIS,
  FL_TK_RPARANTHESIS,
  FL_TK_DOT,
  FL_TK_LBRACKET,
  FL_TK_RBRACKET,
  FL_TK_LCBRACKET,
  FL_TK_RCBRACKET,
  FL_TK_NEW,
  FL_TK_DELETE,
  FL_TK_RESIZE,
  FL_TK_TYPEOF,
  FL_TK_EXIST,
  FL_TK_COLON,
  FL_TK_SEMICOLON,
  FL_TK_EXCLAMATION,
  FL_TK_TILDE,
  FL_TK_SCOMMENT,
  FL_TK_MCOMMENT,
  // FL_TK_MCOMMENT_END,
  FL_TK_ASTERISKEQUAL,
  FL_TK_ASTERISK,
  FL_TK_SLASHEQUAL,
  FL_TK_SLASH,
  FL_TK_MINUSQUAL,
  FL_TK_MINUS,
  FL_TK_MINUS2,
  FL_TK_PLUSEQUAL,
  FL_TK_PLUS,
  FL_TK_PLUS2,
  FL_TK_MODEQUAL,
  FL_TK_MOD,
  FL_TK_GT3EQUAL,
  FL_TK_GT2EQUAL,
  FL_TK_GT,
  FL_TK_GT2,
  FL_TK_GTE,
  FL_TK_LT,
  FL_TK_LT2EQUAL,
  FL_TK_LT2,
  FL_TK_LTE,
  FL_TK_ASSIGNAMENT,
  FL_TK_SASSIGNAMENT,
  FL_TK_EQUAL2,
  FL_TK_TEQUAL,
  FL_TK_EEQUAL,
  FL_TK_AT,
  FL_TK_ARRAY,
  FL_TK_QMARK,
  FL_TK_SQUOTE,
  FL_TK_DQUOTE,
  FL_TK_ANDEQUAL,
  FL_TK_AND,
  FL_TK_AND2,
  FL_TK_CEQUAL,
  FL_TK_CARET,
  FL_TK_OREQUAL,
  FL_TK_OR,
  FL_TK_OR2,
  FL_TK_BOOL,
  FL_TK_TRUE,
  FL_TK_FALSE,
  FL_TK_I8,
  FL_TK_I16,
  FL_TK_I32,
  FL_TK_I64,
  FL_TK_U8,
  FL_TK_U16,
  FL_TK_U32,
  FL_TK_U64,
  FL_TK_F32,
  FL_TK_F64,
  FL_TK_CAST,
  FL_TK_STRING,
  FL_TK_NULL,
  FL_TK_BACKTICK,
  FL_TK_DOLLAR,
  FL_TK_HASH
};

typedef enum fl_tokens fl_tokens_t;

struct fl_tokens_cfg {
  fl_tokens_t type;
  bool can_be_escaped;
  char* text;
  size_t text_s;
  char* close_text;
  size_t close_text_s;
  bool escapable;
};

typedef struct fl_tokens_cfg fl_tokens_cfg_t;

struct fl_token_pos {
  size_t line;
  size_t column;
};

typedef struct fl_token_pos fl_token_pos_t;

struct fl_token {
  string* string;
  fl_tokens_t type;
  fl_token_pos_t start;
  fl_token_pos_t end;
};

typedef struct fl_token fl_token_t;

struct fl_token_list {
  size_t size;
  fl_token_t tokens[];
};

enum fl_parser_result_level {
  FL_PR_ERROR_FINAL,
  FL_PR_WARNING,
  FL_PR_NOTICE,
  FL_PR_ERROR,
};

struct fl_parser_result {
  fl_token_t* token;
  char* text;
  enum fl_parser_result_level level;
};
typedef struct fl_parser_result fl_parser_result_t;

struct fl_parser_state {
  size_t current;
  fl_token_t* token;
  fl_token_t* prev_token;
  fl_token_t* next_token;
  size_t look_ahead_idx;
};

typedef struct fl_parser_state fl_psrstate_t;

enum fl_ast_type {
  FL_AST_STMT_LOG,
  FL_AST_LIT_ARRAY,
  FL_AST_LIT_OBJECT,
  FL_AST_LIT_NUMERIC,
  FL_AST_LIT_STRING,
  FL_AST_LIT_BOOLEAN,
  FL_AST_LIT_NULL,
  FL_AST_LIT_IDENTIFIER,
  FL_AST_EXPR,
};
typedef enum fl_ast_type fl_ast_type_t;

struct fl_ast {
  fl_token_t* token_start;
  fl_token_t* token_end;
  fl_ast_type_t type; // TODO enum

  union {
    struct fl_ast_lit_boolean {
      bool value;
    } boolean;
    struct fl_ast_lit_string {
      // single true, doubles false
      // TODO if support <<<XXX ... XXX; this should be changed to enum
      bool quoted;
      string* value;
    } string;
    struct fl_ast_lit_numeric {
      double value;
    } numeric;
    struct fl_ast_lit_identifier {
      string* string;
    } identifier;
  };
};

typedef struct fl_ast fl_ast_t;

typedef struct fl_token_list fl_token_list_t;

// TODO resize support
struct fl_parser_stack {
  size_t current;
  fl_psrstate_t states[500];
};

typedef struct fl_parser_stack fl_psrstack_t;

//-
//- MACROS
//-

#define FL_READER_DECL(name)                                                   \
  extern fl_ast_t* fl_read_##name(fl_token_list_t* tokens,                     \
                                  fl_psrstack_t* stack, fl_psrstate_t* state);

#define FL_READER_IMPL(name)                                                   \
  fl_ast_t* fl_read_##name(fl_token_list_t* tokens, fl_psrstack_t* stack,      \
                           fl_psrstate_t* state)

// , printf("%s\n", #name)
#define FL_READ(name)                                                          \
  fl_read_##name(tokens, stack, state)

// TODO handle errors when done :)
#define FL_TRY_READ(name)                                                      \
  ast = FL_READ(name);                                                         \
  if (ast) {                                                                   \
    return ast;                                                                \
  }

#define FL_AST_START(ast_type)                                                 \
  fl_ast_t* ast = (fl_ast_t*)malloc(sizeof(fl_ast_t));                         \
  ast->token_start = state->token;                                             \
  ast->type = ast_type;

#define FL_RETURN_AST()                                                        \
  ast->token_end = state->token;                                               \
  return ast;

#define FL_RETURN_NOT_FOUND()                                                  \
  free(ast);                                                                   \
  return 0;

#define FL_ACCEPT(string) fl_parser_accept(tokens, state, string)

#define FL_ACCEPT_TOKEN(token_type)                                            \
fl_parser_accept_token(tokens, state, token_type)

// , printf("next!\n")
#define FL_NEXT() fl_parser_next(tokens, state)

//-
//- functions
//-

extern size_t fl_token_list_count;

extern fl_tokens_cfg_t fl_token_list[];

extern void fl_tokens_init();

extern void fl_tokens_delete(fl_token_list_t* tokens);

/* cldoc:begin-category(parser-stack.c) */

extern void fl_tokens_debug(fl_token_list_t* tokens);

extern fl_token_list_t* fl_tokenize(string* file);

/* cldoc:end-category() */

/* cldoc:begin-category(parser.c) */

extern fl_ast_t* fl_parser(fl_token_list_t* tokens);

extern fl_ast_t* fl_parse(string* str);
extern fl_ast_t* fl_parse_utf8(char* str);

FL_READER_DECL(body);

/* cldoc:end-category() */

/* cldoc:begin-category(parser-utils.c) */

extern bool fl_parser_next(fl_token_list_t* tokens, fl_psrstate_t* state);

extern bool fl_parser_prev(fl_token_list_t* tokens, fl_psrstate_t* state);

extern bool fl_parser_eof(fl_token_list_t* tokens, fl_psrstate_t* state);

extern bool fl_parser_accept(fl_token_list_t* tokens, fl_psrstate_t* state,
                             char* text);

extern bool fl_parser_accept_token(fl_token_list_t* tokens, fl_psrstate_t* state,
                             fl_tokens_t token_type);

/* cldoc:end-category() */

/* cldoc:begin-category(parser-stack.c) */

extern void fl_parser_stack_init(fl_psrstack_t* stack, fl_token_list_t* tokens,
                                 fl_psrstate_t* state);

extern void fl_parser_look_ahead(fl_psrstack_t* stack, fl_psrstate_t* state);

extern void fl_parser_commit(fl_psrstack_t* stack, fl_psrstate_t* state);

extern void fl_parser_rollback(fl_psrstack_t* stack, fl_psrstate_t* state);

extern fl_parser_result_t* fl_parser_expect(fl_token_list_t* tokens,
                                            fl_psrstate_t* state, char* text,
                                            char* err_msg, bool final);

extern void fl_parser_skipws(fl_token_list_t* tokens, fl_psrstate_t* state);

/* cldoc:end-category() */

/* cldoc:begin-category(parser-literal.c) */

FL_READER_DECL(literal);
FL_READER_DECL(lit_null);
FL_READER_DECL(lit_boolean);
FL_READER_DECL(lit_string);
FL_READER_DECL(lit_numeric);
FL_READER_DECL(lit_identifier);

/* cldoc:end-category() */
