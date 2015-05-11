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
#include "stringc.h"

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
  FL_TK_MCOMMENT_START,
  FL_TK_MCOMMENT_END,
  FL_TK_ASTERISK,
  FL_TK_SLASH,
  FL_TK_MINUS,
  FL_TK_MINUS2,
  FL_TK_PLUS,
  FL_TK_PLUS2,
  FL_TK_MOD,
  FL_TK_GT,
  FL_TK_GT2,
  FL_TK_GTE,
  FL_TK_LT,
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
  FL_TK_AND,
  FL_TK_AND2,
  FL_TK_CARET,
  FL_TK_OR,
  FL_TK_OR2,
  FL_TK_BOOL,
  FL_TK_TRUE,
  FL_TK_FALSE,
  FL_TK_I8,
  FL_TK_I16,
  FL_TK_I32,
  FL_TK_I64,
  FL_TK_u8,
  FL_TK_u16,
  FL_TK_u32,
  FL_TK_U64,
  FL_TK_F32,
  FL_TK_F64,
  FL_TK_STRING,
  FL_TK_NULL,
  FL_TK_BACKTICK,
  FL_TK_DOLLAR,
  FL_TK_HASH
};

typedef enum fl_tokens fl_tokens_t;

struct fl_tokens_cfg {
  fl_tokens_t token;
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
  string* value;
  fl_tokens_t token;
  fl_token_pos_t start;
  fl_token_pos_t end;
};

typedef struct fl_token fl_token_t;

struct fl_token_list {
  size_t size;
  fl_token_t tokens[];
};
typedef struct fl_token_list fl_token_list_t;

extern size_t fl_token_list_count;
extern fl_tokens_cfg_t fl_token_list[];

extern void fl_tokens_init();
extern fl_token_list_t* fl_tokenize(string* file);
