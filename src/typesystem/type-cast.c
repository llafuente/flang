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

bool ts_castable(size_t aty_id, size_t bty_id) {
  if (aty_id == bty_id) {
    return true;
  }

  // TODO special cases atm
  if ((aty_id == TS_STRING && bty_id == TS_CSTR) ||
      (aty_id == TS_CSTR && bty_id == TS_STRING)) {
    return true;
  }

  ty_t atype = ts_type_table[aty_id];
  ty_t btype = ts_type_table[bty_id];

  if (btype.of == atype.of) {
    switch (btype.of) {
    case FL_NUMBER:
      // a is bigger & both floating/number
      if (atype.number.bits >= btype.number.bits &&
          btype.number.fp == atype.number.fp) {
        return true; // both numbers
      }
      break;
    default: {}
    }
  }

  printf("\n");
  ty_dump(aty_id);
  printf("\n");
  ty_dump(bty_id);

  log_verbose("invalid cast [%zu] to [%zu]", aty_id, bty_id);

  return false;
}

ast_cast_operations_t ts_cast_operation(ast_t* node) {
  size_t current = node->ty_id;
  size_t expected = node->cast.element->ty_id;

  ty_t cu_type = ts_type_table[current];
  ty_t ex_type = ts_type_table[expected];

  if (ex_type.of == cu_type.of) {
    switch (ex_type.of) {
    case FL_NUMBER:
      // fpto*i
      if (cu_type.number.fp && !ex_type.number.fp) {
        if (ex_type.number.sign) {
          return FL_CAST_FPTOSI;
        }
        return FL_CAST_FPTOUI;
      }
      // *itofp
      if (!cu_type.number.fp && ex_type.number.fp) {
        if (ex_type.number.sign) {
          return FL_CAST_SITOFP;
        }
        return FL_CAST_UITOFP;
      }

      bool fp = cu_type.number.fp;
      // upcast
      if (cu_type.number.bits < ex_type.number.bits) {
        if (fp) {
          return FL_CAST_FPEXT;
        }

        // sign -> sign
        if ((cu_type.number.sign && ex_type.number.sign) ||
            (!cu_type.number.sign && ex_type.number.sign)) {
          return FL_CAST_SEXT;
        }
        return FL_CAST_ZEXT;
      }

      // downcast / truncate
      if (cu_type.number.bits >= ex_type.number.bits) {
        if (fp) {
          return FL_CAST_FPTRUNC;
        }
        return FL_CAST_TRUNC;
      }
    case FL_POINTER:
      // only allow it if both are same type
      if (ts_type_table[ex_type.ptr.to].of ==
          ts_type_table[cu_type.ptr.to].of) {
        return FL_CAST_BITCAST;
      }
    }
  }

  if (cg_bitcast(ex_type, cu_type) || cg_bitcast(cu_type, ex_type)) {
    return FL_CAST_BITCAST;
  }

  // TODO check autocast
  log_warning("incompatible pointers");

  string* name = st_newc("autocast", st_enc_ascii);
  size_t args_ty[1];
  args_ty[0] = expected;
  ast_t* autocast = ast_search_fn_wargs(node, name, args_ty, 1);
  if (autocast) {
    // TODO change cast to expr-call

    ast_t* parent = node->parent;

    PSR_CREATE(arguments, FL_AST_LIST);
    arguments->list.count = 1;
    arguments->list.elements = calloc(1, sizeof(ast_t*));
    arguments->list.elements[0] = node->cast.element;
    node->parent = arguments;

    PSR_CREATE(callee, FL_AST_LIT_IDENTIFIER);
    callee->identifier.string = st_clone(autocast->func.id->identifier.string);
    callee->identifier.resolve = false;
    callee->identifier.decl = autocast;

    PSR_CREATE(ecall, FL_AST_EXPR_CALL);
    ecall->call.arguments = arguments;
    ecall->call.callee = callee;

    callee->parent = ecall;
    arguments->parent = ecall;
    ecall->parent = parent;
    return FL_CAST_AUTO;
  }
  st_delete(&name);

  if (!current || !expected) {
    log_warning("inference is still needed!");
    return 0;
  }

  ty_dump(current);
  printf("\n");
  ty_dump(expected);
  printf("\n");

  log_error("invalid casting");
  return 0;
}

ast_action_t ts_cast_operation_pass_cb(ast_t* node, ast_t* parent, size_t level,
                                       void* userdata_in, void* userdata_out) {
  switch (node->type) {
  case FL_AST_CAST: {
    ts_cast_operation(node);
  }
  }
  return FL_AC_CONTINUE;
}
