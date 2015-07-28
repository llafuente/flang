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

// warning
// IF COUNT IS OUT OF SYNC, BAD THING HAPPEN!

size_t fl_token_list_count = 104;
fl_tokens_cfg_t fl_token_list[104] = {
    {FL_TK_EOF, false, "\x00", 1, 0, 0, false},
    {FL_TK_NEWLINE, false, "\n", 1, 0, 0, false},
    {FL_TK_LOG, false, "log", 3, 0, 0, false},
    {FL_TK_LOG, false, "echo", 4, 0, 0, false},
    {FL_TK_FUNCTION, false, "function", 8, 0, 0, false},
    {FL_TK_FUNCTION, false, "fn", 2, 0, 0, false},
    {FL_TK_FFI_C, false, "ffi", 3, 0, 0, false},
    {FL_TK_RETURN, false, "return", 6, 0, 0, false},
    {FL_TK_VAR, false, "var", 3, 0, 0, false},
    {FL_TK_UNVAR, false, "unvar", 4, 0, 0, false},
    {FL_TK_CONST, false, "const", 4, 0, 0, false},
    {FL_TK_STATIC, false, "static", 5, 0, 0, false},
    {FL_TK_GLOBAL, false, "global", 5, 0, 0, false},
    {FL_TK_LPARENTHESIS, false, "(", 1, 0, 0, false},
    {FL_TK_RPARENTHESIS, false, ")", 1, 0, 0, false},
    {FL_TK_3DOT, false, "...", 3, 0, 0, false},
    {FL_TK_DOT, false, ".", 1, 0, 0, false},
    {FL_TK_LCBRACKET, false, "{", 1, 0, 0, false},
    {FL_TK_RCBRACKET, false, "}", 1, 0, 0, false},
    {FL_TK_NEW, false, "new", 3, 0, 0, false},
    {FL_TK_DELETE, false, "delete", 6, 0, 0, false},
    {FL_TK_RESIZE, false, "resize", 6, 0, 0, false},
    {FL_TK_TYPEOF, false, "typeof", 6, 0, 0, false},
    {FL_TK_EXIST, false, "?", 1, 0, 0, false},
    {FL_TK_COLON, false, ":", 1, 0, 0, false},
    {FL_TK_SEMICOLON, false, ";", 1, 0, 0, false},
    {FL_TK_COMMA, false, ",", 1, 0, 0, false},

    {FL_TK_SCOMMENT, false, "//", 2, "\n", 1, false},
    {FL_TK_MCOMMENT, false, "/*", 2, "*/", 2, false},
    {FL_TK_MCOMMENT, false, "*/", 2, 0, 0, false},
    {FL_TK_ASTERISKEQUAL, false, "*=", 2, 0, 0, false},
    {FL_TK_ASTERISK, false, "*", 1, 0, 0, false},
    {FL_TK_SLASHEQUAL, false, "/=", 2, 0, 0, false},
    {FL_TK_SLASH, false, "/", 1, 0, 0, false},

    {FL_TK_MINUS2, false, "--", 2, 0, 0, false},
    {FL_TK_MINUSQUAL, false, "-=", 2, 0, 0, false},
    {FL_TK_MINUS, false, "-", 1, 0, 0, false},

    {FL_TK_PLUS2, false, "++", 2, 0, 0, false},
    {FL_TK_PLUSEQUAL, false, "+=", 2, 0, 0, false},
    {FL_TK_PLUS, false, "+", 1, 0, 0, false},

    {FL_TK_MODEQUAL, false, "%=", 2, 0, 0, false},
    {FL_TK_MOD, false, "%", 1, 0, 0, false},

    {FL_TK_GT3EQUAL, false, ">>>=", 4, 0, 0, false},
    {FL_TK_GT2EQUAL, false, ">>=", 3, 0, 0, false},
    {FL_TK_GT2, false, ">>", 2, 0, 0, false},
    {FL_TK_GTE, false, ">=", 2, 0, 0, false},
    {FL_TK_GT, false, ">", 1, 0, 0, false},

    {FL_TK_LT2EQUAL, false, "<<=", 3, 0, 0, false},
    {FL_TK_LT2, false, "<<", 2, 0, 0, false},
    {FL_TK_LTE, false, "<=", 2, 0, 0, false},
    {FL_TK_LT, false, "<", 1, 0, 0, false},

    {FL_TK_EQUAL2, false, "==", 2, 0, 0, false},
    {FL_TK_EQUAL, false, "=", 1, 0, 0, false},

    {FL_TK_QMARKEQUAL, false, "?=", 2, 0, 0, false},
    {FL_TK_QMARK, false, "?", 1, 0, 0, false},

    {FL_TK_TEQUAL, false, "~=", 2, 0, 0, false},
    {FL_TK_TILDE, false, "~", 1, 0, 0, false},

    // ≠
    {FL_TK_EEQUAL, false, "!=", 2, 0, 0, false},
    {FL_TK_EXCLAMATION, false, "!", 1, 0, 0, false},

    {FL_TK_CEQUAL, false, "^=", 2, 0, 0, false},
    {FL_TK_CARET, false, "^", 1, 0, 0, false},

    {FL_TK_OREQUAL, false, "|=", 2, 0, 0, false},
    {FL_TK_OR, false, "|", 1, 0, 0, false},
    {FL_TK_OR2, false, "||", 2, 0, 0, false},
    {FL_TK_OR2, false, "or", 2, 0, 0, false},

    {FL_TK_AND2, false, "&&", 2, 0, 0, false},
    {FL_TK_ANDEQUAL, false, "&=", 2, 0, 0, false},
    {FL_TK_AND, false, "&", 1, 0, 0, false},
    {FL_TK_AND2, false, "and", 3, 0, 0, false},

    {FL_TK_ARRAY, false, "[]", 2, 0, 0, false},
    {FL_TK_LBRACKET, false, "[", 1, 0, 0, false},
    {FL_TK_RBRACKET, false, "]", 1, 0, 0, false},

    // strings literals
    {FL_TK_SQUOTE, false, "'", 1, "'", 1, true},
    {FL_TK_DQUOTE, false, "\"", 1, "\"", 1, true},

    // types
    {FL_TK_VOID, false, "void", 4, 0, 0, false},
    {FL_TK_BOOL, false, "bool", 4, 0, 0, false},
    {FL_TK_TRUE, false, "true", 4, 0, 0, false},
    {FL_TK_TRUE, false, "on", 2, 0, 0, false},
    {FL_TK_TRUE, false, "yes", 3, 0, 0, false},
    {FL_TK_FALSE, false, "false", 5, 0, 0, false},
    {FL_TK_FALSE, false, "off", 3, 0, 0, false},
    {FL_TK_FALSE, false, "no", 2, 0, 0, false},
    {FL_TK_I8, false, "i8", 2, 0, 0, false},
    {FL_TK_I16, false, "i16", 3, 0, 0, false},
    {FL_TK_I32, false, "i32", 3, 0, 0, false},
    {FL_TK_I64, false, "i64", 3, 0, 0, false},
    {FL_TK_U8, false, "u8", 2, 0, 0, false},
    {FL_TK_U16, false, "u16", 3, 0, 0, false},
    {FL_TK_U32, false, "u32", 3, 0, 0, false},
    {FL_TK_U64, false, "u64", 3, 0, 0, false},
    {FL_TK_F32, false, "f32", 3, 0, 0, false},
    {FL_TK_F64, false, "f64", 3, 0, 0, false},
    {FL_TK_STRING, false, "string", 6, 0, 0, false},
    {FL_TK_NULL, false, "null", 4, 0, 0, false},
    {FL_TK_NULL, false, "nil", 3, 0, 0, false},

    {FL_TK_IF, false, "if", 2, 0, 0, false},
    {FL_TK_ELSE, false, "else", 4, 0, 0, false},
    {FL_TK_FOR, false, "for ", 4, 0, 0, false}, // TODO nasty hack?
    {FL_TK_WHILE, false, "while", 5, 0, 0, false},
    {FL_TK_DO, false, "do", 2, 0, 0, false},

    {FL_TK_CAST, false, "cast", 4, 0, 0, false},
    // special chars
    {FL_TK_AT, false, "@", 1, 0, 0, false},
    {FL_TK_BACKTICK, false, "`", 1, 0, 0, false},
    {FL_TK_DOLLAR, false, "$", 1, 0, 0, false},
    {FL_TK_HASH, false, "#", 1, 0, 0, 0}};
