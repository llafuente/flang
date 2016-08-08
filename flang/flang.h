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

#include <setjmp.h>
#include <inttypes.h>
#include <stdbool.h> // import bool

#define null 0
#define true 1
#define false 0
//#define bool int8_t
#define i8 int8_t
#define i16 int16_t
#define i32 int32_t
#define i64 int64_t
#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t
#define f32 float
#define f64 double

#define libexport extern
#define internal static

#include "ext/hash.h"  // import hash_t
#include "ext/array.h" // import array_t
#include "ext/pool.h"  // import pool_t
#include "stringc.h"   // import string

// defined @flang-typesystem
enum ts_types;
typedef enum ts_types ts_types_t;

struct ts_type;
typedef struct ts_type ty_t;

struct ts_type_struct_alias;
typedef struct ts_type_struct_alias ts_type_struct_alias_t;

struct ts_struct_alias;
typedef struct ts_struct_alias ts_struct_alias_t;

struct ts_struct_virtual;
typedef struct ts_struct_virtual ts_struct_virtual_t;

enum ts_cast_modes;
typedef enum ts_cast_modes ts_cast_modes_t;

// defined @flang-ast
enum ast_action;
typedef enum ast_action ast_action_t;

enum ast_function_type;
typedef enum ast_function_type ast_function_type_t;

enum ast_cast_operations;
typedef enum ast_cast_operations ast_cast_operations_t;

enum ast_scope;
typedef enum ast_scope ast_scope_t;

enum ast_types;
typedef enum ast_types ast_types_t;

struct ast;
typedef struct ast ast_t;

struct ast;
typedef enum ast_trav_mode ast_trav_mode_t;

// callback type for [ast_traverse](#ast_traverse) & [ast_reverse](#ast_reverse)
typedef ast_action_t (*ast_cb_t)(ast_trav_mode_t mode, ast_t* node,
                                 ast_t* parent, u64 level, void* userdata_in,
                                 void* userdata_out);

extern jmp_buf fl_on_error_jmp;
extern ast_t* ast_last_error_node;
extern char* ast_last_error_message;

libexport void flang_init();
libexport void flang_exit(ast_t* root);
