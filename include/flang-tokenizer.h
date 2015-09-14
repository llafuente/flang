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

struct tk_position {
  size_t line;
  size_t column;
};

enum tk_tokens {
  FL_TK_UNKOWN = 0,
  FL_TK_EOF,
  FL_TK_WHITESPACE,
  FL_TK_NEWLINE,
  FL_TK_LOG,
  FL_TK_FUNCTION,
  FL_TK_FFI_C,
  FL_TK_RETURN,
  FL_TK_VAR,
  FL_TK_UNVAR,
  FL_TK_CONST = 10,
  FL_TK_STATIC,
  FL_TK_GLOBAL,
  FL_TK_LPARENTHESIS,
  FL_TK_RPARENTHESIS,
  FL_TK_3DOT,
  FL_TK_DOT,
  FL_TK_LBRACKET,
  FL_TK_RBRACKET,
  FL_TK_LCBRACKET,
  FL_TK_RCBRACKET = 20,
  FL_TK_EXIST,
  FL_TK_COLON,
  FL_TK_SEMICOLON,
  FL_TK_COMMA,
  FL_TK_EXCLAMATION,
  FL_TK_TILDE,
  FL_TK_SCOMMENT,
  FL_TK_MCOMMENT,
  FL_TK_ASTERISKEQUAL,
  FL_TK_ASTERISK = 30,
  FL_TK_SLASHEQUAL,
  FL_TK_SLASH,
  FL_TK_MINUSQUAL,
  FL_TK_MINUS,
  FL_TK_MINUS2,
  FL_TK_PLUSEQUAL,
  FL_TK_PLUS,
  FL_TK_PLUS2,
  FL_TK_MODEQUAL,
  FL_TK_MOD = 40,
  FL_TK_GT3EQUAL,
  FL_TK_GT2EQUAL,
  FL_TK_GT,
  FL_TK_GT2,
  FL_TK_GTE,
  FL_TK_LT,
  FL_TK_LT2EQUAL,
  FL_TK_LT2,
  FL_TK_LTE,
  FL_TK_EQUAL = 50,
  FL_TK_QMARKEQUAL,
  FL_TK_EQUAL2,
  FL_TK_TEQUAL,
  FL_TK_EEQUAL,
  FL_TK_AT,
  FL_TK_ARRAY,
  FL_TK_QMARK,
  FL_TK_SQUOTE,
  FL_TK_DQUOTE,
  FL_TK_ANDEQUAL = 60,
  FL_TK_AND,
  FL_TK_AND2,
  FL_TK_CEQUAL,
  FL_TK_CARET,
  FL_TK_OREQUAL,
  FL_TK_OR,
  FL_TK_OR2,
  // types
  FL_TK_VOID = 100,
  FL_TK_BOOL,
  FL_TK_TRUE,
  FL_TK_FALSE,
  FL_TK_I8,
  FL_TK_I16,
  FL_TK_I32,
  FL_TK_I64,
  FL_TK_U8,
  FL_TK_U16,
  FL_TK_U32 = 110,
  FL_TK_U64,
  FL_TK_F32,
  FL_TK_F64,
  FL_TK_NULL,
  FL_TK_STRUCT,

  FL_TK_IF = 120,
  FL_TK_ELSE,
  FL_TK_FOR,
  FL_TK_WHILE,
  FL_TK_DO,
  FL_TK_NEW,
  FL_TK_DELETE,
  FL_TK_RESIZE,
  FL_TK_TYPEOF,
  FL_TK_CAST,
  FL_TK_STRING,
  FL_TK_BACKTICK,
  FL_TK_DOLLAR,
  FL_TK_HASH,
  FL_TK_LOAD
};

struct tk_token {
  string* string;
  tk_tokens_t type;
  tk_position_t start;
  tk_position_t end;
};

struct tk_token_list {
  size_t size;
  tk_token_t tokens[];
};

struct tk_state {
  size_t line;
  size_t column;
  char* itr;
  char* end;
};

struct tk_token_cfg {
  tk_tokens_t type;
  char* text;
  size_t text_s;
  char* close_text;
  size_t close_text_s;
  bool escapable;
  bool is_punctuation;
  bool is_reserved_word;
};
