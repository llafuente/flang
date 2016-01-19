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

#define TS_VOID 1
#define TS_BOOL 2
#define TS_U8 3
#define TS_I8 4
#define TS_U16 5
#define TS_I16 6
#define TS_U32 7
#define TS_I32 8
#define TS_U64 9
#define TS_I64 10
#define TS_F32 11
#define TS_F64 12
#define TS_CSTR 13
#define TS_PVOID 14
#define TS_VECI8 15
#define TS_STRING 13 // TODO this should be 16

enum ts_types {
  FL_VOID = 1,
  FL_NUMBER = 2,
  FL_POINTER = 3, // wrapper
  FL_VECTOR = 4,  // wrapper
  FL_FUNCTION = 5,
  FL_STRUCT = 6,
  // FL_ENUM = 7, // TODO this is in fact an "int"

  FL_INFER = 10,

  FL_TEMPLATE = 20,
};

// type must be unique
struct ts_type {
  string* id;
  string* decl;

  ts_types_t of;
  void* codegen; // cache for codegen.

  // string, array, any are implemented inside the language
  // ref must be studied where should be, it's a very special pointer...
  union {
    // void is void :)

    // bool, iX, uX, fX
    struct ts_type_number {
      unsigned char bits;
      bool fp;
      bool sign;
    } number;

    struct ts_type_pointer {
      size_t to;
    } ptr;

    struct ts_type_vector {
      size_t to;
      size_t length;
    } vector;

    struct ts_type_function {
      ast_t* decl;

      size_t ret;
      size_t* params;
      size_t nparams;
      bool varargs;

    } func;

    struct ts_type_struct {
      ast_t* decl;

      size_t* fields;
      string** properties;
      size_t nfields;

    } structure;

    struct ts_type_enum {
      ast_t* decl;

      size_t* members;
      size_t nmembers;

    } enu;

    struct ts_type_template {
      ast_t* decl;
    } tpl;
  };
};
