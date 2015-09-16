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

enum ts_types {
  FL_VOID = 1,
  FL_NUMBER = 2,
  FL_POINTER = 3, // wrapper
  FL_VECTOR = 4,  // wrapper
  FL_FUNCTION = 5,
  FL_STRUCT = 6,
  FL_ENUM = 7, // TODO this is in fact an "int"

  FL_INFER = 10,

  FL_TEMPLATE0 = 20,
  FL_TEMPLATE1 = 21,
  FL_TEMPLATE2 = 22,
  FL_TEMPLATE3 = 23,
  FL_TEMPLATE4 = 24,
  FL_TEMPLATE5 = 25,
  FL_TEMPLATE6 = 26,
  FL_TEMPLATE7 = 27,
  FL_TEMPLATE8 = 28,
  FL_TEMPLATE9 = 29,
  // FL_REFERENCE = 12,
};

// type must be unique
struct ts_type {
  string* id;

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
      size_t ret;
      size_t* params;
      size_t nparams;
      bool varargs;

      ast_t* decl;
    } func;

    struct ts_type_struct {
      size_t* fields;
      size_t nfields;

      ast_t* decl;
    } structure;

    struct ts_type_enum {
      size_t* members;
      size_t nmembers;

      ast_t* decl;
    } enu;
  };
};

struct ts_typeh {
  array list;        // of ast_t* decl;
  UT_hash_handle hh; // makes this structure hashable
  char name[64];     // key (string is WITHIN the structure)
};
