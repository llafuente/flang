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

#include "flang/flang.h"
#include "flang/libts.h"
#include "flang/libparser.h"
#include "src/libparser/grammar/tokens.h"
#include "flang/libast.h"
#include "flang/debug.h"

// TODO this should check if the type can be promoted
// TODO check if struct are compatible
ts_cast_modes_t ts_cast_mode(u64 current, u64 expected) {
  // same obviously true, no type yet also true: no need cast
  if (current == expected || !current) {
    return CAST_NONE;
  }

  // TODO REVIEW special cases atm
  if ((current == TS_STRING && expected == TS_CSTR) ||
      (current == TS_CSTR && expected == TS_STRING)) {
    return CAST_NONE;
  }

  ty_t cur_type = ty(current);
  ty_t ex_type = ty(expected);

  if (cur_type.of == ex_type.of) {
    switch (ex_type.of) {
    case TY_POINTER:
      // void* to anything is ok, to avoid casting malloc
      if (cur_type.ptr.to == TS_VOID) {
        return CAST_IMPLICIT;
      }
      // if the target type are implicit castables, then it's ok
      // fail otherwise
      ts_cast_modes_t cm = ts_cast_mode(cur_type.ptr.to, ex_type.ptr.to);
      if ((cm & 1) == 1) {
        return CAST_IMPLICIT;
      }

      return CAST_INVALID;
      break;
    case TY_NUMBER:
      // b is bigger & both floating/number
      if (cur_type.number.bits <= ex_type.number.bits &&
          ex_type.number.fp == cur_type.number.fp) {
        return CAST_IMPLICIT; // both numbers
      }
      // from integer to float
      if (!cur_type.number.fp && ex_type.number.fp) {
        return CAST_IMPLICIT;
      }
      return CAST_EXPLICIT;
      break;
    default: {}
    }
  }

  // vector-ptr casting
  if (((ex_type.of == TY_POINTER && cur_type.of == TY_VECTOR) ||
       (cur_type.of == TY_POINTER && ex_type.of == TY_VECTOR)) &&
      ex_type.ptr.to == cur_type.vector.to) {
    return CAST_IMPLICIT;
  }

  // pointer to reference
  // printf("%d -> %d '%s'\n", cur_type.of, cur_type.of == TY_REFERENCE,
  // ty_to_string(current)->value);
  // printf("%d -> %d '%s'\n", ex_type.of, ex_type.of == TY_POINTER,
  // ty_to_string(expected)->value);
  // printf("same type %d\n", cur_type.ref.to == ex_type.ptr.to);
  // printf("void %d\n", cur_type.ptr.to == TS_VOID);
  if ((cur_type.of == TY_POINTER && ex_type.of == TY_REFERENCE) &&
      (cur_type.ref.to == ex_type.ptr.to || // same type
       cur_type.ptr.to == TS_VOID           // void pointer. autocast malloc
       )) {
    return CAST_IMPLICIT;
  }

  // TODO REVIEW this maybe break code, could be dangerous
  // if ((cur_type.of == TY_POINTER && ex_type.of == TY_REFERENCE)) {
  //  return cur_type.ref.to == ex_type.ptr.to;
  //}

  log_verbose("invalid cast [%s] to [%s]", ty_to_string(current)->value,
              ty_to_string(expected)->value);

  return CAST_INVALID;
}

bool ts_explicit_cast(u64 current, u64 expected) {
  return ts_cast_mode(current, expected) == CAST_EXPLICIT;
}

bool ts_implicit_cast(u64 current, u64 expected) {
  return ts_cast_mode(current, expected) == CAST_IMPLICIT;
}

bool ts_castable(u64 current, u64 expected) {
  return (ts_cast_mode(current, expected) & 1) == 1;
}

ast_cast_operations_t ts_cast_operation(ast_t* node) {
  fl_assert(node->type == AST_CAST);

  u64 expected = node->ty_id;
  u64 current = node->cast.element->ty_id;
  log_verbose("cast %zu to %zu", current, expected);

  ty_t cu_type = ts_type_table[current];
  ty_t ex_type = ts_type_table[expected];
  log_verbose("same type %d", ex_type.of == cu_type.of);

  if (ex_type.of == cu_type.of) {
    switch (ex_type.of) {
    case TY_NUMBER:
      // fpto*i
      log_verbose("%d && %d", cu_type.number.fp, ex_type.number.fp);

      if (cu_type.number.fp && !ex_type.number.fp) {
        if (ex_type.number.sign) {
          return AST_CAST_FPTOSI;
        }
        return AST_CAST_FPTOUI;
      }
      // *itofp
      if (!cu_type.number.fp && ex_type.number.fp) {
        if (ex_type.number.sign) {
          return AST_CAST_SITOFP;
        }
        return AST_CAST_UITOFP;
      }

      bool fp = cu_type.number.fp;
      // upcast
      if (cu_type.number.bits < ex_type.number.bits) {
        if (fp) {
          return AST_CAST_FPEXT;
        }

        if ((!cu_type.number.sign && !ex_type.number.sign) ||
            (!cu_type.number.sign && ex_type.number.sign)) {
          return AST_CAST_ZEXT;
        }

        return AST_CAST_SEXT;
        /* not necessary :)
        if ((!cu_type.number.sign && !ex_type.number.sign) ||
            (cu_type.number.sign && !ex_type.number.sign)) {
        }
        */
      }

      // downcast / truncate
      if (cu_type.number.bits >= ex_type.number.bits) {
        if (fp) {
          return AST_CAST_FPTRUNC;
        }
        return AST_CAST_TRUNC;
      }
    case TY_POINTER:
      // only allow it if both are same type
      // or one is void
      if (ts_type_table[ex_type.ptr.to].of ==
          ts_type_table[cu_type.ptr.to].of) {
        return AST_CAST_BITCAST;
      }
    default: {} // supress warning
    }
  }

  return AST_CAST_BITCAST;
  /* TODO REVIEW !!!
  if (!current || !expected) {
    log_warning("inference is still needed!");
    return 0;
  }

  ast_raise_error(node,
                  "invalid casting: \x1B[36m%s\x1B[39m to \x1B[36m%s\x1B[39m",
                  ty_to_string(current)->value, ty_to_string(expected)->value);
  */
}

ast_t* __ts_autocast(ast_t* node, u64 input, u64 output) {
  string* name = st_newc("autocast", st_enc_ascii);
  u64 args_ty[1];
  args_ty[0] = input;
  ast_t* autocast = ast_search_fn(node, name, args_ty, 1, output, false);
  st_delete(&name);

  return autocast;
}

bool ts_cast_literal(ast_t* node, u64 type_id) {
  if (node->type == AST_LIT_FLOAT || node->type == AST_LIT_INTEGER) {
    // destination value must also be a number!
    if (!ty_is_number(type_id)) {
      ast_raise_error(node, "Number cannot be casted to: %s",
                      ty_to_string(type_id)->value);
    }
    node->ty_id = type_id;
    return node;
  }
  if (node->type == AST_EXPR_LUNARY && node->lunary.operator== '-') {
    return ts_cast_literal(node->lunary.element, type_id);
  }
  return false;
}

ast_t* __cast_node_to(ast_t* node, u64 type_id) {
  fl_assert(node->ty_id > 0);

  // try to cast to "inference" type, just wait...
  if (!type_id) {
    return node;
  }

  // case 1: if it's a literal, can be casted to a compatible type directly
  if (ts_cast_literal(node, type_id)) {
    log_verbose("literal casted");
    return node;
  }

  // case 2: do nothing, wait until the template is expanded.
  if (ty_is_template(node->ty_id) || ty_is_template(type_id)) {
    return node;
  }

  // case 3: there is a autocast function, cast it!
  string* name = st_newc("autocast", st_enc_ascii);
  u64 args_ty[1];
  args_ty[0] = node->ty_id;
  ast_t* autocast = ast_search_fn(node, name, args_ty, 1, type_id, false);
  st_delete(&name);

  if (autocast) {
    log_verbose("cast to expr-call");

    ast_t* arguments = ast_mk_list();
    ast_mk_list_push(arguments, node);
    node->parent = arguments;

    ast_t* callee = ast_mk_lit_id(st_clone(autocast->func.uid), false);
    callee->identifier.decl = autocast;
    callee->ty_id = autocast->ty_id;

    ast_t* ecall = ast_mk_call_expr(callee, arguments);
    ecall->call.decl = autocast;

    callee->parent = ecall;
    arguments->parent = ecall;
    ecall->parent = node;
    ecall->ty_id = type_id;

    return ecall;
  }

  // case 3: invalid cast, raise
  ts_cast_modes_t cm = ts_cast_mode(node->ty_id, type_id);
  if (cm == CAST_INVALID) {
    ast_raise_error(node, "Invalid cast: types are not castables '%s' to '%s'",
                    ty_to_string(node->ty_id)->value,
                    ty_to_string(type_id)->value);
  }

  // case 4: explicit cast, raise
  if (cm == CAST_EXPLICIT) {
    ast_raise_error(node, "Explicit cast required between '%s' to '%s'",
                    ty_to_string(node->ty_id)->value,
                    ty_to_string(type_id)->value);
  }
  /*
  log_verbose("cast mode? %d", ts_cast_mode(node->ty_id, type_id));
  log_verbose("autocast? %p", __ts_autocast(node, node->ty_id, type_id));

  if (ts_cast_mode(node->ty_id, type_id) == CAST_INVALID &&
      !__ts_autocast(node, node->ty_id, type_id)) {
    // TODO this need to be a bit more specific about what is casting to return
    // a readble error
    ast_raise_error(node,
                    "manual casting is required: '%s' is %s and must be %s",
                    ast_get_code(node)->value, ty_to_string(node->ty_id)->value,
                    ty_to_string(type_id)->value);
    return node; // almost an error!?
  }
  */

  // no error or ignore, so cast :)
  ast_t* cast = ast_mk_cast(0, node, false);
  cast->parent = node->parent;
  node->parent = cast;
  cast->ty_id = type_id;

  return cast;
}

ast_t* __ts_create_left_cast(ast_t* parent, ast_t* left) {
  fl_assert(parent->type == AST_EXPR_BINOP);
  ast_t* cast = __cast_node_to(left, parent->ty_id);
  parent->binop.left = cast;

  return cast;
}

ast_t* __ts_create_right_cast(ast_t* parent, ast_t* right) {
  fl_assert(parent->type == AST_EXPR_BINOP);
  ast_t* cast = __cast_node_to(right, parent->ty_id);
  parent->binop.right = cast;

  return cast;
}

void __ts_create_binop_cast(ast_t* bo) {
  fl_assert(bo->type == AST_EXPR_BINOP);

  ast_t* l = bo->binop.left;
  ast_t* r = bo->binop.right;
  u64 expected_ty_id = bo->ty_id;

  if (expected_ty_id == TS_BOOL) {
    // both must have the same type!
    if (ast_is_literal(l)) {
      ast_t* cast = __cast_node_to(l, r->ty_id);
      bo->binop.left = cast;
      return;
    }

    if (ast_is_literal(r)) {
      ast_t* cast = __cast_node_to(r, l->ty_id);
      bo->binop.right = cast;
      return;
    }

    if (ty_is_number(l->ty_id) && ty_is_number(r->ty_id)) {
      expected_ty_id = ts_promote_typeid(l->ty_id, r->ty_id);
    }
  }

  if (expected_ty_id != l->ty_id) {
    // cast left side
    ast_t* cast = __cast_node_to(l, expected_ty_id);
    bo->binop.left = cast;
  }

  if (expected_ty_id != r->ty_id) {
    ast_t* cast = __cast_node_to(r, expected_ty_id);
    bo->binop.right = cast;
  }
}

void ts_cast_return(ast_t* node) {
  fl_assert(node->type == AST_STMT_RETURN);

  if (node->ty_id)
    return;

  ast_t* decl = node->parent;
  while (decl->parent && decl->type != AST_DECL_FUNCTION) {
    decl = decl->parent;
  }

  if (!decl) {
    ast_raise_error(node, "syntax error, return found outside function scope");
  }

  ast_t* arg = node->ret.argument;
  ts_pass(arg);

  u64 t = decl->func.ret_type->ty_id;
  if (t != node->ret.argument->ty_id) {
    ast_t* cast = __cast_node_to(arg, t);
    node->ret.argument = cast;
    node->ty_id = cast->ty_id;
  } else {
    node->ty_id = t;
  }
}

void ts_cast_lunary(ast_t* node) {
  switch (node->lunary.operator) {
  case '!':
    node->ty_id = 2; // bool
    break;
  case '*': { // dereference, pointer/reference
    ast_t* el = node->lunary.element;
    ts_pass(el);

    if (!ty_is_pointer_like(el->ty_id)) {
      ast_raise_error(el, "type cannot be dereferenced: %s",
                      ty_to_string(el->ty_id)->value);
    }

    node->ty_id = ty(el->ty_id).ptr.to;
  } break;
  case '&': {
    ast_t* el = node->lunary.element;
    ts_pass(el);
    node->ty_id = ty_create_wrapped(TY_POINTER, el->ty_id);
  } break;
  default:
    ts_pass(node->lunary.element);
    node->ty_id = node->lunary.element->ty_id;
  }
}

void ts_cast_runary(ast_t* node) {
  ts_pass(node->runary.element);
  node->ty_id = node->runary.element->ty_id;
}

void ts_cast_call(ast_t* node) {
  fl_assert(node->type == AST_EXPR_CALL);

  if (node->ty_id) {
    return;
  }

  // TODO this fail when calling a member
  log_debug("call '%s' ty_id[%zu]", node->call.callee->identifier.string->value,
            node->ty_id);

  u64 i;
  ast_t* args = node->call.arguments;
  ast_t* arg;
  u64 count = args->list.length;

  // get types from arguments first
  log_debug("callee and arguments must pass first!");
  ts_pass(node->call.callee);
  for (i = 0; i < count; ++i) {
    ts_pass(args->list.values[i]);
  }

  // NOTE: polymorph - callee must be an identifier

  // no-identifier, check compatibility
  u64 cty_id = node->call.callee->ty_id;
  log_debug("call current ty_id[%lu]", cty_id);
  if (cty_id) {
    // this happend when calle it's not a literal
    // check if it's compatible
    if (!ty_compatible_fn(cty_id, args, false, false)) {
      ast_raise_error(node, "Incompatible call arguments expected: %s",
                      ty_to_string(cty_id)->value);
      return;
    }
  } else {
    // callee it's an identifier, marked as resolved: false
    // due to polymorph we cannot use normal method: ast_search_id*
    ast_t* tmp =
        ast_search_fn_wargs(node->call.callee->identifier.string, args);
    if (!tmp) {
      // TODO error should be more specific
      ast_raise_error(node, "Incompatible call arguments");
      return;
    }

    if (tmp->func.templated) {
      tmp = ast_implement_fn(node->call.arguments, tmp, 0);
      log_silly("fn expanded: %zu", tmp->ty_id);
    }
    cty_id = tmp->ty_id;
    log_silly("set callee ty_id[%lu]", node->call.callee->ty_id);
  }

  if (!cty_id) {
    log_warning("ignore expr call type");
    return; // TODO passthought printf atm
  }

  node->call.callee->ty_id = cty_id;

  ast_t* decl = ts_type_table[cty_id].func.decl;
  node->call.decl = decl;

  node->ty_id = ts_type_table[cty_id].func.ret;
  log_silly("set return ty_id[%lu]", node->ty_id);

  ty_t* t = &ts_type_table[cty_id];
  fl_assert(t->of == TY_FUNCTION);

  // cast arguments
  log_verbose("varargs[%d] params[%zu] args[%zu]", t->func.varargs,
              t->func.nparams, count);
  for (i = 0; i < count; ++i) {
    arg = args->list.values[i];

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
      args->list.values[i] = __cast_node_to(arg, t->func.params[i]);
    }
  }
}

ast_t* __ts_dereference(ast_t* node) {
  log_silly("left: auto-dereference");
  ast_t* deref = ast_mk_lunary(node, '*');
  node->parent = deref;
  ts_pass(deref);

  return deref;
}

void ts_cast_binop(ast_t* node) {
  fl_assert(node->type == AST_EXPR_BINOP || node->type == AST_EXPR_ASSIGNAMENT);

  log_debug("binop found %d", node->binop.operator);
  // cast if necessary
  ast_t* l = node->binop.left;
  ast_t* r = node->binop.right;

  // operation that need casting or fp/int
  ts_pass(l);
  ts_pass(r);

  u64 l_type = l->ty_id;
  u64 r_type = r->ty_id;

  // left: auto-dereference if right is not a pointer or type compatible
  if (ty_is_reference(l_type) && ty_is_reference(r_type)) {
    // ast_dump_s(node);
    // fl_assert(false); // TODO REVIEW need study
  }
  if (ty_is_reference(l_type)) {
    if (!ty_is_pointer_like(r_type) ||
        ts_cast_mode(ty(l_type).ref.to, r_type) != CAST_INVALID) {
      ast_t* deref = __ts_dereference(l);
      deref->parent = node;
      node->binop.left = deref;
      l_type = deref->ty_id;
    }
  } else if (ty_is_reference(r_type)) {
    // right: auto-dereference when type compatible
    if (ts_cast_mode(ty(r_type).ref.to, l_type) != CAST_INVALID) {
      ast_t* deref = __ts_dereference(r);
      deref->parent = node;
      node->binop.right = deref;
      r_type = deref->ty_id;
    }
  }

  log_verbose("l_type %zu r_type %zu", l_type, r_type);

  bool l_fp = ty_is_fp(l_type);
  bool r_fp = ty_is_fp(r_type);

  switch (node->binop.operator) {
  // assignament
  case '=':
  case TK_SHLEQ:
  case TK_SHREQ:
  case TK_MINUSEQ:
  case TK_ANDEQ:
  case TK_OREQ:
  case TK_PLUSEQ:
  case TK_STAREQ:
  case TK_SLASHEQ:
  case TK_CARETEQ:
  case TK_PERCENTEQ: {
    fl_assert(node->type == AST_EXPR_ASSIGNAMENT);

    if (l_type != r_type) {
      log_silly("assignament cast [%zu - %zu]", l_type, r_type);
      // cast will be validated later if it's posible
      node->binop.right = __cast_node_to(r, l_type);
    }

    node->ty_id = l_type;
    return;
  } break;
  case '-':
  case '+': {
    // pointer arithmetic

    // left is a pointer, right must be a number
    if (ty_is_pointer(l_type)) {
      if (!ty_is_number(r_type)) {
        ast_raise_error(
            node,
            "Invalid operants: left is (%s) but right is not numeric (%s).",
            ty_to_string(l_type)->value, ty_to_string(r_type)->value);
      }
      node->ty_id = l_type;
      return;
    }
    // left is a number and right is a pointer
    // normalize: put the pointer on the left side
    if (ty_is_pointer(r_type)) {
      // lhs must be a numeric type
      if (!ty_is_number(l_type)) {
        ast_raise_error(
            node,
            "Invalid operants: right is (%s) but left is not numeric (%s).",
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
  case '%':
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

    if (!ty_is_number(l_type) || !ty_is_number(r_type)) {
      // one of each is not a number, so need to be operator overloaded
      // get all operator overloading available for left type
      ast_t* fn = ast_search_fn_op(node, node->binop.operator, l_type);

      if (!fn) {
        ast_raise_error(node,
                        "cannot find a proper overloading function for %s",
                        psr_operator_str(node->binop.operator));
      }

      // transform binop into function call
      ast_t* arguments = ast_mk_list();
      ast_mk_list_push(arguments, l);
      ast_mk_list_push(arguments, r);

      // clear
      ast_clear(node, AST_EXPR_CALL);
      node->call.callee = fn->func.id;
      node->call.arguments = arguments;
      node->call.decl = fn;

      // NOTE this is needed to handle all types, and no create unnecesary
      // castings
      _typesystem(node);

      log_verbose("operator overloading: binop to expr call.");
    } else if ((l_static && r_static) || (!l_static && !r_static)) {
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
  fl_assert(node->type == AST_EXPR_MEMBER);

  if (node->ty_id)
    return;

  ast_t* l = node->member.left;
  ast_t* p = node->member.property;

  u64 l_typeid;
  ts_pass(l);

  log_debug("l->ty_id = %zu", l->ty_id);

  // now we should know left type
  // get poperty index -> typeid
  // TODO perf
  ty_t type = ty(l->ty_id);
  switch (type.of) {
  case TY_STRUCT: {
    if (node->member.brakets) {
      // operator overloading TK_ACCESS
      ast_t* fn = ast_search_fn_op(node, TK_ACCESS, l->ty_id);

      if (!fn) {
        ast_raise_error(node,
                        "cannot find a proper overloading function for []");
      }

      // transform binop into function call
      ast_t* arguments = ast_mk_list();
      ast_mk_list_push(arguments, l);
      ast_mk_list_push(arguments, p);

      // clear
      ast_clear(node, AST_EXPR_CALL);
      node->call.callee = fn->func.id;
      node->call.arguments = arguments;
      node->call.decl = fn;

      // NOTE this is needed to handle all types, and no create unnecesary
      // castings
      _typesystem(node);

      log_verbose("operator overloading: binop to expr call.");

    } else {
      p->ty_id = node->ty_id =
          ty_get_struct_prop_type(l->ty_id, p->identifier.string);
      node->member.idx = ty_get_struct_prop_idx(l->ty_id, p->identifier.string);

      if (node->member.idx == -1) {
        // lookup for a function property

        ast_t* fn_property =
            ty_get_virtual(l->ty_id, p->identifier.string, false);

        if (!fn_property) {
          // lookup and implement
          ty_t type2 = ty(l->ty_id);
          if (type2.structure.from_tpl) {
            fn_property = ty_get_virtual(type2.structure.from_tpl,
                                         p->identifier.string, false);

            ast_t* type_list = ast_mk_list();
            ast_mk_list_push(type_list, l);

            fn_property = ast_implement_fn(type_list, fn_property, 0);

            // implement
            log_silly("need to implement!")
          }

          if (!fn_property) {
            // not found, error!
            ast_raise_error(node, "invalid member access '%s' for struct: %s",
                            ast_get_code(p)->value,
                            ty_to_string(l->ty_id)->value);
          }
        }

        // ty_get_virtual(u64 ty_id, string* id)

        // transform member-access to expr-call
        ast_clear(node, AST_EXPR_CALL);
        node->call.callee = fn_property->func.id;

        ast_t* arguments = ast_mk_list();
        ast_mk_list_push(arguments, l);
        node->call.arguments = arguments;

        node->call.decl = fn_property;
        node->ty_id = fn_property->func.ret_type->ty_id;
      }
    }
  } break;
  case TY_POINTER: {
    node->ty_id = type.ptr.to;
    node->member.property = __cast_node_to(p, 9);
  } break;
  case TY_VECTOR: {
    node->ty_id = type.vector.to;
  } break;
  default: { ast_raise_error(node, "Invalid member access type"); }
  }
}
