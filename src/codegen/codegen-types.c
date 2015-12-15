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

// helpers class, call it twice swaping args!
bool cg_bitcast(ty_t a, ty_t b) {
  if (a.of == FL_VECTOR && b.of == FL_POINTER && a.vector.to == b.ptr.to) {
    return true;
  }

  // both pointers, one is void
  // TODO recursive ?
  if (b.of == FL_POINTER && b.ptr.to == TS_VOID) {
    return true;
  }
  if (a.of == FL_POINTER && a.ptr.to == TS_VOID) {
    return true;
  }

  return false;
}

LLVMTypeRef cg_get_type(ast_t* node, LLVMContextRef context) {
  return cg_get_typeid(node, node->ty_id, context);
}

LLVMTypeRef cg_get_typeid(ast_t* node, size_t id, LLVMContextRef context) {
  ty_t* t = &ts_type_table[id];

  if (t->codegen) {
    return (LLVMTypeRef)t->codegen;
  }

  log_verbose("llvm for typeid = %zu", id);

  switch (t->of) {
  case FL_VOID:
    // t->codegen = (void*)LLVMVoidType();
    // NOTE by recomendation of llvm use i8
    t->codegen = (void*)LLVMIntType(8);
    break;
  case FL_NUMBER:
    if (t->number.fp) {
      switch (t->number.bits) {
      case 32:
        t->codegen = (void*)LLVMFloatType();
        break;
      case 64:
        t->codegen = (void*)LLVMDoubleType();
        break;
      }
    } else {
      log_silly("t.number.bits %d", t->number.bits);

      t->codegen = (void*)LLVMIntType(t->number.bits);
    }
    break;
  case FL_POINTER:
    t->codegen =
        (void*)LLVMPointerType(cg_get_typeid(node, t->ptr.to, context), 0);
    break;
  case FL_STRUCT: {
    log_verbose("codegen struct '%s'", t->id->value);
    t->codegen = LLVMStructCreateNamed(context, t->id->value);
    // create the list!
    ast_t* l = t->structure.decl->structure.fields;

    size_t i;
    size_t count = t->structure.nfields;
    LLVMTypeRef* types = malloc(count * sizeof(LLVMTypeRef));
    for (i = 0; i < count; ++i) {
      // types[i] = cg_get_typeid(l->list.elements[i]->ty_id, context);
      types[i] = cg_get_typeid(node, t->structure.fields[i], context);
    }
    LLVMStructSetBody(t->codegen, types, count, 0);
    free(types);
  } break;
  case FL_VECTOR: {
    t->codegen = LLVMArrayType(cg_get_typeid(node, t->vector.to, context),
                               t->vector.length);
  } break;
  case FL_FUNCTION: {
    LLVMTypeRef params[t->func.nparams];
    size_t i;
    for (i = 0; i < t->func.nparams; ++i) {
      params[i] = cg_get_typeid(node, t->func.params[i], context);
    }

    t->codegen = LLVMFunctionType(cg_get_typeid(node, t->func.ret, context),
                                  params, t->func.nparams, t->func.varargs);
    break;
  }
  default: {
    ast_raise_error(node, "invalid type [%s] typesystem fail",
                    ty_to_string(node->ty_id)->value);
  }
  }

  if (!t->codegen) {
    log_error("cannot find LLVM-type");
  }

  return (LLVMTypeRef)t->codegen;
}
