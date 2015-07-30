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

size_t fl_token_list_count = 106;
fl_tokens_cfg_t fl_token_list[106] = {
    {FL_TK_EOF, "\x00", 1, 0, 0, false, true, false},
    {FL_TK_NEWLINE, "\n", 1, 0, 0, false, true, false},
    {FL_TK_LOG, "log", 3, 0, 0, false, false, false},
    {FL_TK_LOG, "echo", 4, 0, 0, false, false, false},
    {FL_TK_FUNCTION, "function", 8, 0, 0, false, false, true},
    {FL_TK_FUNCTION, "fn", 2, 0, 0, false, false, true},
    {FL_TK_FFI_C, "ffi", 3, 0, 0, false, false, true},
    {FL_TK_RETURN, "return", 6, 0, 0, false, false, true},
    {FL_TK_VAR, "var", 3, 0, 0, false, false, true},
    {FL_TK_UNVAR, "unvar", 4, 0, 0, false, false, true},
    {FL_TK_CONST, "const", 4, 0, 0, false, false, true},
    {FL_TK_STATIC, "static", 5, 0, 0, false, false, true},
    {FL_TK_GLOBAL, "global", 5, 0, 0, false, false, true},
    {FL_TK_LPARENTHESIS, "(", 1, 0, 0, false, true, false},
    {FL_TK_RPARENTHESIS, ")", 1, 0, 0, false, true, false},
    {FL_TK_3DOT, "...", 3, 0, 0, false, true, false},
    {FL_TK_DOT, ".", 1, 0, 0, false, true, false},
    {FL_TK_LCBRACKET, "{", 1, 0, 0, false, true, false},
    {FL_TK_RCBRACKET, "}", 1, 0, 0, false, true, false},
    {FL_TK_NEW, "new", 3, 0, 0, false, false, true},
    {FL_TK_DELETE, "delete", 6, 0, 0, false, false, true},
    {FL_TK_RESIZE, "resize", 6, 0, 0, false, false, true},
    {FL_TK_TYPEOF, "typeof", 6, 0, 0, false, false, true},
    {FL_TK_EXIST, "?", 1, 0, 0, false, true, false},
    {FL_TK_COLON, ":", 1, 0, 0, false, true, false},
    {FL_TK_SEMICOLON, ";", 1, 0, 0, false, true, false},
    {FL_TK_COMMA, ",", 1, 0, 0, false, true, false},

    {FL_TK_SCOMMENT, "//", 2, "\n", 1, false, true, false},
    {FL_TK_MCOMMENT, "/*", 2, "*/", 2, false, true, false},
    {FL_TK_MCOMMENT, "*/", 2, 0, 0, false, true, false},
    {FL_TK_ASTERISKEQUAL, "*=", 2, 0, 0, false, true, false},
    {FL_TK_ASTERISK, "*", 1, 0, 0, false, true, false},
    {FL_TK_SLASHEQUAL, "/=", 2, 0, 0, false, true, false},
    {FL_TK_SLASH, "/", 1, 0, 0, false, true, false},

    {FL_TK_MINUS2, "--", 2, 0, 0, false, true, false},
    {FL_TK_MINUSQUAL, "-=", 2, 0, 0, false, true, false},
    {FL_TK_MINUS, "-", 1, 0, 0, false, true, false},

    {FL_TK_PLUS2, "++", 2, 0, 0, false, true, false},
    {FL_TK_PLUSEQUAL, "+=", 2, 0, 0, false, true, false},
    {FL_TK_PLUS, "+", 1, 0, 0, false, true, false},

    {FL_TK_MODEQUAL, "%=", 2, 0, 0, false, true, false},
    {FL_TK_MOD, "%", 1, 0, 0, false, true, false},

    {FL_TK_GT3EQUAL, ">>>=", 4, 0, 0, false, true, false},
    {FL_TK_GT2EQUAL, ">>=", 3, 0, 0, false, true, false},
    {FL_TK_GT2, ">>", 2, 0, 0, false, true, false},
    {FL_TK_GTE, ">=", 2, 0, 0, false, true, false},
    {FL_TK_GT, ">", 1, 0, 0, false, true, false},

    {FL_TK_LT2EQUAL, "<<=", 3, 0, 0, false, true, false},
    {FL_TK_LT2, "<<", 2, 0, 0, false, true, false},
    {FL_TK_LTE, "<=", 2, 0, 0, false, true, false},
    {FL_TK_LT, "<", 1, 0, 0, false, true, false},

    {FL_TK_EQUAL2, "==", 2, 0, 0, false, true, false},
    {FL_TK_EQUAL, "=", 1, 0, 0, false, true, false},

    {FL_TK_QMARKEQUAL, "?=", 2, 0, 0, false, true, false},
    {FL_TK_QMARK, "?", 1, 0, 0, false, true, false},

    {FL_TK_TEQUAL, "~=", 2, 0, 0, false, true, false},
    {FL_TK_TILDE, "~", 1, 0, 0, false, true, false},

    // ≠
    {FL_TK_EEQUAL, "!=", 2, 0, 0, false, true, false},
    {FL_TK_EXCLAMATION, "!", 1, 0, 0, false, true, false},

    {FL_TK_CEQUAL, "^=", 2, 0, 0, false, true, false},
    {FL_TK_CARET, "^", 1, 0, 0, false, true, false},

    {FL_TK_OREQUAL, "|=", 2, 0, 0, false, true, false},
    {FL_TK_OR, "|", 1, 0, 0, false, true, false},
    {FL_TK_OR2, "||", 2, 0, 0, false, true, false},
    {FL_TK_OR2, "or", 2, 0, 0, false, true, false},

    {FL_TK_AND2, "&&", 2, 0, 0, false, true, false},
    {FL_TK_ANDEQUAL, "&=", 2, 0, 0, false, true, false},
    {FL_TK_AND, "&", 1, 0, 0, false, true, false},
    {FL_TK_AND2, "and", 3, 0, 0, false, true, false},

    {FL_TK_ARRAY, "[]", 2, 0, 0, false, true, false},
    {FL_TK_LBRACKET, "[", 1, 0, 0, false, true, false},
    {FL_TK_RBRACKET, "]", 1, 0, 0, false, true, false},

    // strings literals
    {FL_TK_SQUOTE, "'", 1, "'", 1, true, true, false},
    {FL_TK_DQUOTE, "\"", 1, "\"", 1, true, true, false},

    // types
    {FL_TK_VOID, "void", 4, 0, 0, false, false, true},
    {FL_TK_BOOL, "bool", 4, 0, 0, false, false, true},
    {FL_TK_TRUE, "true", 4, 0, 0, false, false, true},
    {FL_TK_TRUE, "on", 2, 0, 0, false, false, true},
    {FL_TK_TRUE, "yes", 3, 0, 0, false, false, true},
    {FL_TK_FALSE, "false", 5, 0, 0, false, false, true},
    {FL_TK_FALSE, "off", 3, 0, 0, false, false, true},
    {FL_TK_FALSE, "no", 2, 0, 0, false, false, true},
    {FL_TK_I8, "i8", 2, 0, 0, false, false, true},
    {FL_TK_I16, "i16", 3, 0, 0, false, false, true},
    {FL_TK_I32, "i32", 3, 0, 0, false, false, true},
    {FL_TK_I64, "i64", 3, 0, 0, false, false, true},
    {FL_TK_U8, "u8", 2, 0, 0, false, false, true},
    {FL_TK_U16, "u16", 3, 0, 0, false, false, true},
    {FL_TK_U32, "u32", 3, 0, 0, false, false, true},
    {FL_TK_U64, "u64", 3, 0, 0, false, false, true},
    {FL_TK_F32, "f32", 3, 0, 0, false, false, true},
    {FL_TK_F64, "f64", 3, 0, 0, false, false, true},
    {FL_TK_STRING, "string", 6, 0, 0, false, false, true},
    {FL_TK_NULL, "null", 4, 0, 0, false, false, true},
    {FL_TK_NULL, "nil", 3, 0, 0, false, false, true},

    {FL_TK_IF, "if", 2, 0, 0, false, false, true},
    {FL_TK_ELSE, "else", 4, 0, 0, false, false, true},
    {FL_TK_FOR, "for", 3, 0, 0, false, false, true},
    {FL_TK_WHILE, "while", 5, 0, 0, false, false, true},
    {FL_TK_DO, "do", 2, 0, 0, false, false, true},

    {FL_TK_STRUCT, "struct", 6, 0, 0, false, false, true},
    {FL_TK_CAST, "cast", 4, 0, 0, false, false, true},
    // special chars
    {FL_TK_AT, "@", 1, 0, 0, false, true, false},
    {FL_TK_BACKTICK, "`", 1, 0, 0, false, true, false},
    {FL_TK_DOLLAR, "$", 1, 0, 0, false, true, false},
    {FL_TK_HASH, "#", 1, 0, 0, false, true, false}};
