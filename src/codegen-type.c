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
    printf("number (%dbits) fp%d sign%d\n", t.number.bits, t.number.fp,
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
      printf("t.number.bits %d\n", t.number.bits);

      t.codegen = (void*)LLVMIntType(t.number.bits);
    }
    break;
  case FL_POINTER:
    t.codegen = (void*)LLVMPointerType(fl_codegen_get_typeid(t.ptr.to), 0);
    break;
  default:
    printf("not handled type yet.\n");
  }

  if (!t.codegen) {
    printf("cannot find LLVM-type.\n");
  }

  return (LLVMTypeRef)t.codegen;
}

LLVMValueRef fl_codegen_cast_op(LLVMBuilderRef builder, size_t current,
                                size_t expected, LLVMValueRef value) {
  printf("*** casting ***\n");
  printf("%zu == %zu\n", expected, current);

  if (expected == current) {
    return value;
  }

  fl_type_t ex_type = fl_type_table[current];
  fl_type_t cu_type = fl_type_table[expected];

  if (ex_type.of == cu_type.of) {
    switch (ex_type.of) {
    case FL_NUMBER:
      // fptosi
      if (cu_type.number.fp && !ex_type.number.fp) {
        if (ex_type.number.sign) {
          return LLVMBuildFPToSI(builder, value,
                                 fl_codegen_get_typeid(expected), "cast");
        }

        return LLVMBuildFPToUI(builder, value, fl_codegen_get_typeid(expected),
                               "cast");
      }

      // sitofp
      if (!cu_type.number.fp && ex_type.number.fp) {
        if (ex_type.number.sign) {
          return LLVMBuildSIToFP(builder, value,
                                 fl_codegen_get_typeid(expected), "cast");
        }

        return LLVMBuildUIToFP(builder, value, fl_codegen_get_typeid(expected),
                               "cast");
      }

      // LLVMBuildFPTrunc

      bool fp = cu_type.number.fp;

      // upcast
      if (cu_type.number.bits < ex_type.number.bits) {
        if (fp) {
          return LLVMBuildFPExt(builder, value, fl_codegen_get_typeid(expected),
                                "cast");
        }

        // sign -> sign
        if ((cu_type.number.sign && ex_type.number.sign) ||
            (!cu_type.number.sign && ex_type.number.sign)) {
          return LLVMBuildSExt(builder, value, fl_codegen_get_typeid(expected),
                               "cast");
        }

        return LLVMBuildZExt(builder, value, fl_codegen_get_typeid(expected),
                             "cast");
      }

      // downcast / truncate
      if (cu_type.number.bits > ex_type.number.bits) {
        if (fp) {
          return LLVMBuildFPTrunc(builder, value,
                                  fl_codegen_get_typeid(expected), "cast");
        }
        return LLVMBuildTrunc(builder, value, fl_codegen_get_typeid(expected),
                              "cast");
      }
      break;
    }
  }

  fprintf(stderr, "invalid casting");
  exit(1);
  // LLVMBuildSIToFP(LLVMBuilderRef B, LLVMValueRef Val, LLVMTypeRef DestTy,
  // const char *Name)
}
