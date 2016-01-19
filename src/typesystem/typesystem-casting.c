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

// TODO this should check if the type can be promoted
bool ts_castable(size_t current, size_t expected) {
  // same obviously true, no type yet also true
  if (current == expected || !current) {
    return true;
  }

  // TODO special cases atm
  if ((current == TS_STRING && expected == TS_CSTR) ||
      (current == TS_CSTR && expected == TS_STRING)) {
    return true;
  }

  ty_t atype = ts_type_table[current];
  ty_t btype = ts_type_table[expected];

  if (btype.of == atype.of) {
    switch (btype.of) {
    case FL_POINTER:
      // void* to anything is ok, to avoid casting malloc
      // anything else bad
      if (atype.ptr.to == TS_VOID) {
        return true;
      }
      break;
    case FL_NUMBER:
      // b is bigger & both floating/number
      if (atype.number.bits <= btype.number.bits &&
          btype.number.fp == atype.number.fp) {
        return true; // both numbers
      }
      // from integer to float
      if (!atype.number.fp && btype.number.fp) {
        return true;
      }
      break;
    default: {}
    }
  }

  // vector-ptr casting
  if ((btype.of == FL_POINTER && atype.of == FL_VECTOR)) {
    return btype.ptr.to == atype.vector.to;
  }
  if ((atype.of == FL_POINTER && btype.of == FL_VECTOR)) {
    return atype.ptr.to == btype.vector.to;
  }

  log_verbose("invalid cast [%s] to [%s]", ty_to_string(current)->value,
              ty_to_string(expected)->value);

  return false;
}

ast_cast_operations_t ts_cast_operation(ast_t* node) {
  assert(node->type == FL_AST_CAST);

  size_t expected = node->ty_id;
  size_t current = node->cast.element->ty_id;
  log_verbose("cast %zu to %zu", current, expected);

  ty_t cu_type = ts_type_table[current];
  ty_t ex_type = ts_type_table[expected];
  log_verbose("same type %d", ex_type.of == cu_type.of);

  if (ex_type.of == cu_type.of) {
    switch (ex_type.of) {
    case FL_NUMBER:
      // fpto*i
      log_verbose("%d && %d", cu_type.number.fp, ex_type.number.fp);

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

        if ((!cu_type.number.sign && !ex_type.number.sign) ||
            (!cu_type.number.sign && ex_type.number.sign)) {
          return FL_CAST_ZEXT;
        }

        return FL_CAST_SEXT;
        /* not necessary :)
        if ((!cu_type.number.sign && !ex_type.number.sign) ||
            (cu_type.number.sign && !ex_type.number.sign)) {
        }
        */
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
      // or one is void
      if (ts_type_table[ex_type.ptr.to].of ==
          ts_type_table[cu_type.ptr.to].of) {
        return FL_CAST_BITCAST;
      }
    default: {} // supress warning
    }
  }
  log_verbose("cg_bitcast: %d == %d", cg_bitcast(ex_type, cu_type),
              cg_bitcast(cu_type, ex_type));
  if (cg_bitcast(ex_type, cu_type) || cg_bitcast(cu_type, ex_type)) {
    return FL_CAST_BITCAST;
  }

  string* name = st_newc("autocast", st_enc_ascii);
  size_t args_ty[1];
  args_ty[0] = current;
  ast_t* autocast = ast_search_fn(node, name, args_ty, 1, expected, false);
  st_delete(&name);

  if (autocast) {
    log_verbose("cast to expr-call");

    ast_t* arguments = ast_mk_list();
    ast_mk_list_push(arguments, node->cast.element);
    node->parent = arguments;

    ast_t* callee = ast_mk_lit_id(st_clone(autocast->func.uid), false);
    callee->identifier.decl = autocast;

    ast_t* ecall = ast_mk_call_expr(callee, arguments);
    ecall->call.decl = autocast;

    callee->parent = ecall;
    arguments->parent = ecall;
    ecall->parent = node;
    ecall->ty_id = expected;
    node->cast.element = ecall;

    return FL_CAST_AUTO;
  }

  if (!current || !expected) {
    log_warning("inference is still needed!");
    return 0;
  }

  ast_raise_error(node,
                  "invalid casting: \x1B[36m%s\x1B[39m to \x1B[36m%s\x1B[39m",
                  ty_to_string(current)->value, ty_to_string(expected)->value);

  return 0;
}

ast_t* __ts_autocast(ast_t* node, size_t input, size_t output) {
  string* name = st_newc("autocast", st_enc_ascii);
  size_t args_ty[1];
  args_ty[0] = input;
  ast_t* autocast = ast_search_fn(node, name, args_ty, 1, output, false);
  st_delete(&name);

  return autocast;
}

bool ts_cast_literal(ast_t* node, size_t type_id) {
  if (node->type == FL_AST_LIT_FLOAT || node->type == FL_AST_LIT_INTEGER) {
    node->ty_id = type_id;
    return node;
  }
  if (node->type == FL_AST_EXPR_LUNARY && node->lunary.operator== '-') {
    return ts_cast_literal(node->lunary.element, type_id);
  }
  return false;
}

ast_t* __ts_create_cast(ast_t* node, size_t type_id) {
  // try to cast to "inference" type, just wait...
  if (!type_id) {
    return node;
  }

  // literals are always castables
  if (ts_cast_literal(node, type_id)) {
    log_verbose("literal casted");
    return node;
  }

  // do nothing, wait until the template is expanded.
  if (ty_is_template(node->ty_id) || ty_is_template(type_id)) {
    return node;
  }

  log_verbose("castable? %d", ts_castable(node->ty_id, type_id));
  log_verbose("autocast? %p", __ts_autocast(node, node->ty_id, type_id));

  if (!ts_castable(node->ty_id, type_id) &&
      !__ts_autocast(node, node->ty_id, type_id)) {
    ast_raise_error(node,
                    "manual casting is required: '%s' is %s and must be %s",
                    ast_get_code(node)->value, ty_to_string(node->ty_id)->value,
                    ty_to_string(type_id)->value);
    return node; // almost an error!?
  }
  ast_t* cast = ast_mk_cast(0, node);
  cast->parent = node->parent;
  node->parent = cast;
  cast->ty_id = type_id;

  return cast;
}

ast_t* __ts_create_left_cast(ast_t* parent, ast_t* left) {
  assert(parent->type == FL_AST_EXPR_BINOP);
  ast_t* cast = __ts_create_cast(left, parent->ty_id);
  parent->binop.left = cast;

  return cast;
}

ast_t* __ts_create_right_cast(ast_t* parent, ast_t* right) {
  assert(parent->type == FL_AST_EXPR_BINOP);
  ast_t* cast = __ts_create_cast(right, parent->ty_id);
  parent->binop.right = cast;

  return cast;
}

void __ts_create_binop_cast(ast_t* bo) {
  assert(bo->type == FL_AST_EXPR_BINOP);

  ast_t* l = bo->binop.left;
  ast_t* r = bo->binop.right;
  size_t expected_ty_id = bo->ty_id;

  if (expected_ty_id == TS_BOOL) {
    // both must have the same type!
    if (ast_is_literal(l)) {
      ast_t* cast = __ts_create_cast(l, r->ty_id);
      bo->binop.left = cast;
      return;
    }

    if (ast_is_literal(r)) {
      ast_t* cast = __ts_create_cast(r, l->ty_id);
      bo->binop.right = cast;
      return;
    }

    expected_ty_id =
        ts_promote_typeid(bo->binop.left->ty_id, bo->binop.right->ty_id);
  } else {
    expected_ty_id = bo->ty_id;
  }

  if (expected_ty_id != l->ty_id) {
    // cast left side
    ast_t* cast = __ts_create_cast(l, expected_ty_id);
    bo->binop.left = cast;
  }

  if (expected_ty_id != r->ty_id) {
    ast_t* cast = __ts_create_cast(r, expected_ty_id);
    bo->binop.right = cast;
  }
}

void ts_cast_return(ast_t* node) {
  assert(node->type == FL_AST_STMT_RETURN);

  ast_t* decl = node->parent;
  while (decl->parent && decl->type != FL_AST_DECL_FUNCTION) {
    decl = decl->parent;
  }

  if (!decl) {
    log_error("return statement found outside function scope");
  }

  ast_t* arg = node->ret.argument;
  ts_pass(arg);

  size_t t = decl->func.ret_type->ty_id;
  if (t != node->ret.argument->ty_id) {
    ast_t* cast = __ts_create_cast(arg, t);
    node->ret.argument = cast;
  }
}

void ts_cast_lunary(ast_t* node) {
  switch (node->lunary.operator) {
  case '!':
    node->ty_id = 2; // bool
    break;
  case '&': {
    ast_t* el = node->lunary.element;
    ts_pass(el);
    node->ty_id = ty_create_wrapped(FL_POINTER, el->ty_id);
  } break;
  default:
    ts_pass(node->lunary.element);
    node->ty_id = node->lunary.element->ty_id;
  }
}

void ts_cast_assignament(ast_t* node) {
  assert(node->type == FL_AST_EXPR_ASSIGNAMENT);

  ast_t* l = node->assignament.left;
  ast_t* r = node->assignament.right;

  ts_pass(l);
  ts_pass(r);

  size_t l_type = l->ty_id;
  size_t r_type = r->ty_id;

  if (l_type != r_type) {
    log_silly("assignament cast [%zu - %zu]", l_type, r_type);
    // cast will be validated later if it's posible
    node->assignament.right = __ts_create_cast(r, l_type);
  }

  node->ty_id = l_type;
}

void ts_cast_call(ast_t* node) {
  assert(node->type == FL_AST_EXPR_CALL);

  log_debug("call type: %zu", node->ty_id);
  if (node->ty_id) {
    return;
  }

  size_t i;
  ast_t* args = node->call.arguments;
  ast_t* arg;
  size_t count = args->list.count;

  // get types from arguments first
  log_debug("callee and arguments must pass first!");
  ts_pass(node->call.callee);
  for (i = 0; i < count; ++i) {
    ts_pass(args->list.elements[i]);
  }

  // NOTE: polymorph - callee must be an identifier

  size_t cty_id = node->call.callee->ty_id;
  if (cty_id) {
    // this happend when calle it's not a literal
    // check if it's compatible
    if (!ty_compatible_fn(cty_id, args, false, false)) {
      ast_raise_error(node, "Incompatible call arguments");
      return;
    }
  } else {
    // callee it's an identifier, marked as resolved: false
    // due to polymorph we cannot use normal method: ast_search_id*
    ast_t* tmp =
        ast_search_fn_wargs(node->call.callee->identifier.string, args);
    if (!tmp) {
      ast_raise_error(node, "Incompatible call arguments");
      return;
    }
    node->call.callee->ty_id = cty_id = tmp->ty_id;
  }

  if (!cty_id) {
    log_warning("ignore expr call type");
    return; // TODO passthought printf atm
  }

  ast_t* decl = ts_type_table[cty_id].func.decl;
  node->call.decl = decl;
  node->ty_id = ts_type_table[cty_id].func.ret;

  ty_t* t = &ts_type_table[cty_id];
  assert(t->of == FL_FUNCTION);

  // cast arguments
  log_verbose("varargs[%d] params[%zu] args[%zu]", t->func.varargs,
              t->func.nparams, count);
  for (i = 0; i < count; ++i) {
    arg = args->list.elements[i];

    // do not cast varags
    /* TODO ??
    if (t->func.varargs && t->func.nparams == i) {
      return;
    }*/

    if (t->func.nparams == i) {
      break;
    }

    if (arg->ty_id != t->func.params[i]) {
      // cast right side
      log_debug("cast argument %zu != %zu", arg->ty_id, t->func.params[i]);
      args->list.elements[i] = __ts_create_cast(arg, t->func.params[i]);
    }
  }
}

void ts_cast_binop(ast_t* node) {
  assert(node->type == FL_AST_EXPR_BINOP);

  log_debug("binop found %d", node->binop.operator);
  // cast if necessary
  ast_t* l = node->binop.left;
  ast_t* r = node->binop.right;

  // operation that need casting or fp/int
  ts_pass(l);
  ts_pass(r);

  size_t l_type = l->ty_id;
  size_t r_type = r->ty_id;

  log_verbose("l_type %zu r_type %zu", l_type, r_type);

  bool l_fp = ty_is_fp(l_type);
  bool r_fp = ty_is_fp(r_type);

  // here handle pointer math '+' & '-'
  // also normalize the tree for codegen, pointer on the left
  switch (node->binop.operator) {
  case '-':
  case '+': {
    if (ty_is_pointer(l_type)) {
      // rhs must be a numeric type
      if (!ty_is_number(r_type)) {
        ast_raise_error(
            node,
            "Invalid operants. left is (%s) but right is not numeric (%s).",
            ty_to_string(l_type)->value, ty_to_string(r_type)->value);
      }
      node->ty_id = l_type;
      return;
    }
    if (ty_is_pointer(r_type)) {
      // lhs must be a numeric type
      if (!ty_is_number(l_type)) {
        ast_raise_error(
            node,
            "Invalid operants. right is (%s) but left is not numeric (%s).",
            ty_to_string(r_type)->value, ty_to_string(l_type)->value);
      }
      node->binop.right = l;
      node->binop.left = r;
      node->ty_id = r_type;
      return;
    }
  }
  }
  // binop
  switch (node->binop.operator) {
  case TK_EQEQ:
  case TK_NE: // !=
  case TK_LE:
  case '<':
  case TK_GE:
  case '>': {
    // TODO this should test if any side is a literal
    // TEST parser-expression-test.c:187

    // both sides must be the same! the bigger one
    node->ty_id = TS_BOOL; // ts_promote_typeid(l_type, r_type);
    __ts_create_binop_cast(node);
  } break;
  case '&':
  case '|':
  case '^':
  case TK_SHL:
  case TK_SHR:
    // left and right must be Integers!
    if (l_fp || r_fp) {
      ast_raise_error(node, "Invalid operants. Both must be integers.");
    }
  // fallthrough
  default: {
    bool l_static = ast_is_static(l);
    bool r_static = ast_is_static(r);

    log_verbose("static %d == %d", l_static, r_static);

    if ((l_static && r_static) || (!l_static && !r_static)) {
      node->ty_id = ts_promote_typeid(l_type, r_type);
      log_verbose("bigger! %zu", node->ty_id);
      __ts_create_binop_cast(node);

    } else if (l_static) {
      node->ty_id = r_type;
      __ts_create_left_cast(node, l);
    } else if (r_static) {
      node->ty_id = l_type;
      __ts_create_right_cast(node, r);
    }
  }
  }
}

void ts_cast_expr_member(ast_t* node) {
  assert(node->type == FL_AST_EXPR_MEMBER);

  if (node->ty_id)
    return;

  ast_t* l = node->member.left;
  ast_t* p = node->member.property;

  size_t l_typeid;
  ts_pass(l);

  log_debug("l->ty_id = %zu", l->ty_id);

  // now we should know left type
  // get poperty index -> typeid
  // TODO perf
  ty_t* type = &ts_type_table[l->ty_id];
  switch (type->of) {
  case FL_STRUCT: {
    node->ty_id = ty_get_struct_prop_type(l->ty_id, p->identifier.string);
    node->member.idx = ty_get_struct_prop_idx(l->ty_id, p->identifier.string);
  } break;
  case FL_POINTER: {
    node->ty_id = type->ptr.to;
    node->member.property = __ts_create_cast(p, 9);
  } break;
  case FL_VECTOR: {
    node->ty_id = type->vector.to;
  } break;
  default: { ast_raise_error(node, "invalid member access type"); }
  }
}
