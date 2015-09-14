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

void ty_dump(size_t ty_id, int indent) {
  ty_t ty = ts_type_table[ty_id];

  switch (ty.of) {
  case FL_VOID:
    log_debug("%*s [%zu] VOID", indent, " ", ty_id);
    break;
  case FL_NUMBER:
    log_debug("%*s [%zu] Number (fp %d, bits %d, sign %d)", indent, " ", ty_id,
              ty.number.fp, ty.number.bits, ty.number.sign);
    break;
  case FL_POINTER:
    log_debug("%*s [%zu] Pointer -> %zu", indent, " ", ty_id, ty.ptr.to);
    ty_dump(ty.ptr.to, indent + 2);
    break;
  case FL_VECTOR:
    log_debug("%*s [%zu] Vector -> %zu", indent, " ", ty_id, ty.ptr.to);
    ty_dump(ty.vector.to, indent + 2);
    break;
  case FL_STRUCT: {
    log_debug("%*s [%zu] Struct [%s]", indent, " ", ty_id,
              ty.structure.decl->structure.id->identifier.string->value);
    size_t i;
    for (i = 0; i < ty.structure.nfields; ++i) {
      ty_dump(ty.structure.fields[i], indent + 2);
    }
  } break;
  case FL_FUNCTION: {
    log_debug("%*s Function [%s] arity(%zu) -> [%zu]", indent, " ",
              ty.id ? ty.id->value : "Anonymous", ty.func.nparams, ty.func.ret);
    size_t i;
    ty_dump(ty.func.ret, indent + 2);
    for (i = 0; i < ty.func.nparams; ++i) {
      ty_dump(ty.func.params[i], indent + 2);
    }
  } break;
  case FL_INFER: {
    log_debug("%*s Unknown", indent, " ");
  } break;
  default: { log_error("ty_dump(%u) not implement", ty.of); }
  }
}

void ty_dump_table() {
  size_t i;

  for (i = 0; i < ts_type_size_s; ++i) {
    ty_dump(i, 0);
  }
}
