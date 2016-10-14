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
#include "flang/libast.h"
#include "flang/debug.h"

ast_action_t __trav_casting(ast_trav_mode_t mode, ast_t* node, ast_t* parent,
                            u64 level, void* userdata_in, void* userdata_out) {
  ++node->ts_passes;

  switch (node->type) {
  // perf: types decl don't need to be casted.
  case AST_TYPE:
  // TODO attrbute may need to be resolved somehow/sometime
  case AST_ATTRIBUTE:
  case AST_DECL_STRUCT: {
    return AST_SEARCH_SKIP;
  }
  // do not allow assignaments inside test-expressions
  case AST_STMT_IF: {
    ts_check_no_assignament(node->if_stmt.test);
  } break;
  case AST_STMT_LOOP: {
    if (node->loop.pre_cond != 0) {
      ts_check_no_assignament(node->loop.pre_cond);
    }

    if (node->loop.post_cond != 0) {
      ts_check_no_assignament(node->loop.post_cond);
    }
  } break;
  // do not pass typesystem to templates
  // templates are incomplete an raise many errors
  case AST_DECL_FUNCTION: {
    // add the virtual to the list in the type, only once
    if (node->func.type == AST_FUNC_PROPERTY && node->ts_passes == 1) {
      ts_pass(node->func.params);
      ty_struct_add_virtual(node);
    }

    if (node->func.type == AST_FUNC_OPERATOR && node->ts_passes == 1 &&
        !node->func.templated) {
      ts_pass(node->func.params);
      ts_check_operator_overloading(node);
    }
    return node->func.templated ? AST_SEARCH_SKIP : AST_SEARCH_CONTINUE;
  }

  case AST_STMT_RETURN: {
    if (mode == AST_TRAV_ENTER) {
      ts_pass(node->ret.argument);
      ts_cast_return(node);
    }
  } break;
  case AST_LIT_STRING: {
    node->ty_id = TS_STRING;
  } break;
  case AST_LIT_IDENTIFIER: {
    if (!node->ty_id) {
      log_debug("search id: '%s' resolve:%d", node->identifier.string->value,
                node->identifier.resolve);

      if (node->identifier.resolve) {
        ast_t* decl = node->identifier.decl =
            ast_scope_decl(node, node->identifier.string);
        if (!decl) {
          ast_mindump(ast_get_root(node));
          ast_raise_error(node, "type error, cannot find declaration for: '%s'",
                          node->identifier.string->value);
          return AST_SEARCH_STOP;
        }

        // it's a var, copy type
        if (decl->type == AST_DECL_FUNCTION) {
          if (decl->func.templated) {
            fl_assert(false); // this should not happen!
          }

          node->ty_id = node->identifier.decl->ty_id;
        } else {
          node->ty_id = ast_get_typeid(node->identifier.decl);
        }
      }
    }

    if (node->parent->type == AST_DTOR_VAR) {
      log_debug("parent is a dtor: fn=%d", ty_is_function(node->ty_id));
      node->parent->ty_id = node->ty_id;
      if (ty_is_templated(node->ty_id)) {
        ast_raise_error(
            node, "type error, cannot declare a variable with a templated type",
            node->identifier.string->value);
      }
    }

  } break;
  case AST_EXPR_MEMBER: {
    if (mode == AST_TRAV_LEAVE) {
      ts_pass(node->member.left);
      ts_cast_expr_member(node);
    }
  } break;
  case AST_EXPR_LUNARY: {
    if (mode == AST_TRAV_LEAVE) {
      ts_pass(node->lunary.element);
      ts_cast_lunary(node);
    }
  } break;
  case AST_EXPR_RUNARY: {
    if (mode == AST_TRAV_LEAVE) {
      ts_cast_runary(node);
    }
  } break;
  case AST_EXPR_CALL: {
    if (!node->call.safe_arguments) {
      ts_check_no_assignament(node->call.arguments);
    }

    if (mode == AST_TRAV_LEAVE) {
      ts_cast_call(node);
    }
  } break;
  case AST_IMPLEMENT: {
    return AST_SEARCH_SKIP;
  }
  case AST_EXPR_ASSIGNAMENT:
  case AST_EXPR_BINOP: {
    if (mode == AST_TRAV_LEAVE) {
      ts_pass(node->binop.left);
      ts_pass(node->binop.right);

      ts_cast_binop(node);
    }
  } break;
  case AST_EXPR_TYPEOF: {
    if (mode == AST_TRAV_LEAVE) {
      ts_pass(node->tof.expr);
      node->ty_id = node->tof.expr->ty_id;
    }
  } break;
  case AST_EXPR_SIZEOF: {
    node->ty_id = TS_I64;
  } break;
  case AST_NEW: {
    if (mode == AST_TRAV_LEAVE) {
      ts_pass(node->new.expr);
      if (node->new.expr->ty_id) {
        // type must be a pointerlike!
        if (!ty_is_pointer_like(node->new.expr->ty_id)) {
          ast_raise_error(node, "type error, new requires a pointer-like type");
        }

        u64 alloc_ty_id = ty(node->new.expr->ty_id).ref.to;

        ast_t* expr = node->new.expr;

        // transform ast_new into expr-call(s)
        ast_clear(node, AST_LIST);
        array_newcap((array*)&node->list, 2);

        ast_t* arguments = ast_mk_list();
        ast_t* type = ast_mk_type(0, 0);
        type->ty_id = alloc_ty_id;
        ast_mk_list_push(arguments, ast_mk_sizeof(type));
        ast_t* callee = ast_mk_lit_id(st_newc("malloc", st_enc_ascii), false);
        ast_t* expr_call = ast_mk_call_expr(callee, arguments);
        // ident = malloc
        ast_t* assignament = ast_mk_assignament(expr, '=', expr_call);
        ast_mk_list_push(node, assignament);

        // TODO REVIEW this call should be optional / removable...
        arguments = ast_mk_list();
        ast_mk_list_push(arguments, ast_clone(expr));
        char* buffer = malloc(32);
        sprintf(buffer, "operator_%d", TK_NEW);
        callee = ast_mk_lit_id(st_newc(buffer, st_enc_ascii), false);
        free(buffer);
        expr_call = ast_mk_call_expr(callee, arguments);
        ast_mk_list_push(node, expr_call);

        ast_parent(node);
        ts_pass(node);
      }
    }
  } break;
  case AST_DELETE: {
    if (mode == AST_TRAV_LEAVE) {
      ts_pass(node->new.expr);
      if (node->new.expr->ty_id) {
        ast_t* expr = node->new.expr;

        // transform ast_new into expr-call(s)
        ast_clear(node, AST_LIST);
        array_newcap((array*)&node->list, 2);

        // TODO REVIEW this call should be optional / removable...
        // call operator free
        ast_t* arguments = ast_mk_list();
        ast_mk_list_push(arguments, expr);

        char* buffer = malloc(32);
        sprintf(buffer, "operator_%d", TK_DELETE);
        ast_t* callee = ast_mk_lit_id(st_newc(buffer, st_enc_ascii), false);
        free(buffer);
        ast_t* expr_call = ast_mk_call_expr(callee, arguments);
        ast_mk_list_push(node, expr_call);

        // call free
        arguments = ast_mk_list();
        ast_mk_list_push(
            arguments, ast_mk_cast(ast_mk_type_pvoid(), ast_clone(expr), true));
        callee = ast_mk_lit_id(st_newc("free", st_enc_ascii), false);
        expr_call = ast_mk_call_expr(callee, arguments);
        ast_mk_list_push(node, expr_call);

        ast_parent(node);
        ts_pass(node);
      }
    }
  } break;
  default: {} // supress warning
  }
  return AST_SEARCH_CONTINUE;
}

ast_action_t __ts_cast_operation_pass_cb(ast_trav_mode_t mode, ast_t* node,
                                         ast_t* parent, u64 level,
                                         void* userdata_in,
                                         void* userdata_out) {
  fl_assert(node != 0);

  if (mode == AST_TRAV_LEAVE)
    return 0;

  if (node->type == AST_CAST && !node->cast.unsafe) {
    // check if it's possible to cast those types
    if (!ts_castable(node->cast.element->ty_id, node->ty_id)) {
      ast_raise_error(
          node,
          "type error, invalid cast: types are not castables (%s) to (%s)",
          ty_to_string(node->cast.element->ty_id)->value,
          ty_to_string(node->ty_id)->value);
    }
    node->cast.operation = ts_cast_operation(node);
  }
  return AST_SEARCH_CONTINUE;
}

ast_t* ts_pass(ast_t* node) {
  // TODO REVIEW this should be enabled...
  // if (node->ty_id) return node;

  ts_inference(node);

  log_debug("typesystem pass start");
  // first create casting
  ast_traverse(node, __trav_casting, 0, 0, 0, 0);

  // validate casting, and assign a valid operation
  ast_traverse(node, __ts_cast_operation_pass_cb, 0, 0, 0, 0);

  // inference again now that we have more info
  if (ts_inference(node)) {
    return ts_pass(node);
  }

  log_debug("typesystem passed");

  return node; // TODO this should be the error
}
