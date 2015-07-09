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

bool ts_is_number(size_t id) {
  fl_type_t t = fl_type_table[id];
  return t.of == FL_NUMBER;
}

bool ts_is_fp(size_t id) {
  fl_type_t t = fl_type_table[id];
  return t.of == FL_NUMBER ? t.number.fp : false;
}

bool ts_is_int(size_t id) {
  fl_type_t t = fl_type_table[id];
  return t.of == FL_NUMBER ? !t.number.fp : false;
}

size_t ts_get_bigger_typeid(size_t a, size_t b) {
  fl_type_t t_a = fl_type_table[a];
  fl_type_t t_b = fl_type_table[b];

  if (t_a.of == FL_NUMBER && t_b.of == FL_NUMBER) {
    // check floating point
    if (t_a.number.fp && !t_b.number.fp) {
      return a;
    }

    if (t_b.number.fp && !t_a.number.fp) {
      return b;
    }
    // check different sign
    if (t_b.number.sign != t_a.number.sign) {
      // TODO grow type ?
      return b;
    }

    // check bits
    return t_a.number.bits > t_b.number.bits ? a : b;
  }

  // only numbers?
  cg_print("(typesystem) ts_get_bigger_typeid of %d & %d\n", t_a.of, t_b.of);

  return a;
}
