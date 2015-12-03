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

ty_t* ts_type_table = 0;
size_t ts_type_size_s = 0;
ts_type_hash_t* ts_hashtable = 0;

// 0 infer
// 1-12 built-in
// 13-x core
// x... user
void ts_init() {
  if (!ts_type_table) {
    ts_type_table = calloc(sizeof(ty_t), 100);

    // 0 means infer!
    ts_type_table[0].of = FL_INFER;
    // [1] void
    ts_type_table[TS_VOID].of = FL_VOID;
    ts_type_table[TS_VOID].id = st_newc("void", st_enc_ascii);

    // [2] bool
    ts_type_table[TS_BOOL].of = FL_NUMBER;
    ts_type_table[TS_BOOL].id = st_newc("bool", st_enc_ascii);
    ts_type_table[TS_BOOL].number.bits = 1;
    ts_type_table[TS_BOOL].number.fp = false;
    ts_type_table[TS_BOOL].number.sign = false;
    // [3-10] i8,u8,i16,u16,i32,u32,i64,u64
    size_t id = 2;
    size_t i = 3;
    char buffer[20];
    for (; i < 7; i++) {
      size_t bits = pow(2, i);
      ts_type_table[++id].of = FL_NUMBER;
      ts_type_table[id].number.bits = bits;
      ts_type_table[id].number.fp = false;
      ts_type_table[id].number.sign = false;
      sprintf(buffer, "u%zu", bits);
      ts_type_table[id].id = st_newc(buffer, st_enc_ascii);

      ts_type_table[++id].of = FL_NUMBER;
      ts_type_table[id].number.bits = bits;
      ts_type_table[id].number.fp = false;
      ts_type_table[id].number.sign = true;
      sprintf(buffer, "i%zu", bits);
      ts_type_table[id].id = st_newc(buffer, st_enc_ascii);
    }

    // [11] f32
    ts_type_table[++id].of = FL_NUMBER;
    ts_type_table[id].id = st_newc("f32", st_enc_ascii);
    ts_type_table[id].number.bits = 32;
    ts_type_table[id].number.fp = true;
    ts_type_table[id].number.sign = true;

    // [12] f64
    ts_type_table[++id].of = FL_NUMBER;
    ts_type_table[id].id = st_newc("f64", st_enc_ascii);
    ts_type_table[id].number.bits = 64;
    ts_type_table[id].number.fp = true;
    ts_type_table[id].number.sign = true;

    // [13] C-str (null-terminated)
    ts_type_table[++id].of = FL_POINTER;
    ts_type_table[id].id = st_newc("cstr", st_enc_ascii);
    ts_type_table[id].ptr.to = TS_I8;

    // [14] ptr void
    ts_type_table[++id].of = FL_POINTER;
    ts_type_table[id].id = st_newc("ptr<void>", st_enc_ascii);
    ts_type_table[id].ptr.to = TS_VOID;

    // [15] vector<i8>
    ts_type_table[++id].of = FL_VECTOR;
    ts_type_table[id].id = st_newc("vector<i8>", st_enc_ascii);
    ts_type_table[id].ptr.to = TS_I8;

    // transfer list ownership
    /*
    // add it!
    string* idstr = st_newc("string", st_enc_utf8);
    size_t* fields = calloc(length, sizeof(size_t));
    ts_type_table[++id].of = FL_STRUCT;
    ts_type_table[id].id = idstr;
    ts_type_table[id].structure.decl = null;
    ts_type_table[id].structure.fields = fields;
    ts_type_table[id].structure.nfields = length;
    ty_create_named(idstr, decl, i);
    */

    // adding types here
    // affects: typesystem pass (some are hardcoded)
    // also affects: ts_exit st_delete(xx.id)

    // [15+] core + user
    ts_type_size_s = ++id;
  }
}

void ts_exit() {
  size_t i;

  for (i = 0; i < ts_type_size_s; ++i) {
    if (i < 16) {
      st_delete(&ts_type_table[i].id);
    }
    // struct and same length?
    if (ts_type_table[i].of == FL_STRUCT) {
      free(ts_type_table[i].structure.fields);
    } else if (ts_type_table[i].of == FL_FUNCTION) {
      free(ts_type_table[i].func.params);
    }
  }

  free(ts_type_table);
  ts_type_table = 0;

  ts_type_hash_t* s;
  ts_type_hash_t* tmp;
  HASH_ITER(hh, ts_hashtable, s, tmp) {
    HASH_DEL(ts_hashtable, s);
    array_delete(&s->list);
    free(s);
  }

  ts_hashtable = 0;
}
