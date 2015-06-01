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

// priv debug
void print_type(fl_type_t t) {
  switch (t.of) {
  case FL_NUMBER:
    printf("number (%dbits) fp%d signed%d\n", t.number.bits, t.number.fp,
           t.number.sign);
    break;
  }
}

LLVMTypeRef fl_codegen_get_type(fl_ast_t* node) {
  return fl_codegen_get_typeid(node->ty.id);
}

LLVMTypeRef fl_codegen_get_typeid(size_t id) {
  // TODO codegen is cached?
  fl_type_t t = fl_type_table[id];

  if (t.codegen) {
    return (LLVMTypeRef)t.codegen;
  }

  printf("(codegen) typeid %zu\n", id);

  switch (t.of) {
  case FL_VOID:
    t.codegen = (void*)LLVMVoidType();
    break;
  case FL_NUMBER:
    if (t.number.fp) {
      switch (t.number.bits) {
      case 32:
        t.codegen = (void*)LLVMFloatType();
        break;
      case 64:
        t.codegen = (void*)LLVMDoubleType();
        break;
      }
    } else {
      t.codegen = (void*)LLVMIntType(t.number.bits);
    }
    break;
  case FL_POINTER:
    t.codegen = (void*)LLVMPointerType(fl_codegen_get_typeid(t.ptr.to), 0);
    break;
  default:
    printf("not handled type yet.");
  }

  if (!t.codegen) {
    printf("cannot find LLVM-type");
  }

  return (LLVMTypeRef)t.codegen;
}
