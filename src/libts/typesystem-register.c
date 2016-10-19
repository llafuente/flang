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

u64 __ts_string_to_tyid(ast_t* node) {
  fl_assert(node != 0);
  fl_assert(node->type == AST_TYPE);

  // empty var_decl for example
  if (!node->ty.id) {
    return 0;
  }
  // once set do not modify, unless you want it
  // like when implementing a struct templated
  if (node->ty_id) {
    return node->ty_id;
  }

  string* t_str = node->ty.id->identifier.string;
  char* tcstr = t_str->value;

  // built-in
  if (strcmp(tcstr, "auto") == 0) {
    return node->ty.id->ty_id = node->ty_id = 0; // inference
  }
  if (strcmp(tcstr, "bool") == 0) {
    return node->ty.id->ty_id = node->ty_id = TS_BOOL;
  }
  if (strcmp(tcstr, "void") == 0) {
    return node->ty.id->ty_id = node->ty_id = TS_VOID;
  }
  if (strcmp(tcstr, "i8") == 0) {
    return node->ty.id->ty_id = node->ty_id = TS_I8;
  }
  if (strcmp(tcstr, "u8") == 0) {
    return node->ty.id->ty_id = node->ty_id = TS_U8;
  }
  if (strcmp(tcstr, "i16") == 0) {
    return node->ty.id->ty_id = node->ty_id = TS_I16;
  }
  if (strcmp(tcstr, "u16") == 0) {
    return node->ty.id->ty_id = node->ty_id = TS_U16;
  }
  if (strcmp(tcstr, "i32") == 0) {
    return node->ty.id->ty_id = node->ty_id = TS_I32;
  }
  if (strcmp(tcstr, "u32") == 0) {
    return node->ty.id->ty_id = node->ty_id = TS_U32;
  }
  if (strcmp(tcstr, "i64") == 0) {
    return node->ty.id->ty_id = node->ty_id = TS_I64;
  }
  if (strcmp(tcstr, "u64") == 0) {
    return node->ty.id->ty_id = node->ty_id = TS_U64;
  }
  if (strcmp(tcstr, "f32") == 0) {
    return node->ty.id->ty_id = node->ty_id = TS_F32;
  }
  if (strcmp(tcstr, "f64") == 0) {
    return node->ty.id->ty_id = node->ty_id = TS_F64;
  }
  if (strcmp(tcstr, "string") == 0) {
    // return node->ty.id->ty_id = node->ty_id = TS_CSTR; // TODO TS_STRING
    // return node->ty.id->ty_id = node->ty_id = TS_STRING;
    return node->ty.id->ty_id = node->ty_id = 16;
  }
  if (strcmp(tcstr, "ptrdiff") == 0) {
    return node->ty.id->ty_id = node->ty_id = TS_PTRDIFF;
  }

  // type below are wrappers, this means they can appear inside a struct
  // pointing to themself, so if our parent is a struct, we should use auto
  // an in the second pass auto will be replaced with the struct type

  if (strcmp(tcstr, "ptr") == 0) {
    fl_assert(node->ty.children != 0);
    fl_assert(node->ty.children->list.length == 1); // TODO raise
    ts_register_types(node->ty.children);

    u64 t = __ts_string_to_tyid(node->ty.children->list.values[0]);
    if (!t && !ast_has_parent(node, AST_DECL_STRUCT))
      return 0;
    return node->ty.id->ty_id = node->ty_id = ty_create_wrapped(TY_POINTER, t);
  }

  if (strcmp(tcstr, "vector") == 0) {
    fl_assert(node->ty.children != 0);
    fl_assert(node->ty.children->list.length == 1); // TODO raise
    ts_register_types(node->ty.children);

    u64 t = __ts_string_to_tyid(node->ty.children->list.values[0]);
    if (!t && !ast_has_parent(node, AST_DECL_STRUCT))
      return 0;
    return node->ty.id->ty_id = node->ty_id = ty_create_wrapped(TY_VECTOR, t);
  }

  if (strcmp(tcstr, "ref") == 0) {
    fl_assert(node->ty.children != 0);
    fl_assert(node->ty.children->list.length == 1); // TODO raise
    ts_register_types(node->ty.children);

    u64 t = __ts_string_to_tyid(node->ty.children->list.values[0]);
    if (!t && !ast_has_parent(node, AST_DECL_STRUCT))
      return 0;
    return node->ty.id->ty_id = node->ty_id =
               ty_create_wrapped(TY_REFERENCE, t);
  }

  log_silly("search type for '%s'", tcstr);
  ast_t* scope = node;
  ast_t* el = node;

  array* scopes = ast_get_scopes(node);
  if (!scopes) {
    log_warning("delayed type '%s'", tcstr);
    return 0;
  }

  for (u64 i = 0; i < scopes->length; ++i) {
    scope = ((ast_t*)scopes->values[i]);

    el = hash_get(scope->block.types, tcstr);
    if (el != 0) {
      // check if it's a struct with templates, in wich case, we need to
      // implement / reuse
      if (el->type == AST_DECL_STRUCT && el->structure.tpls != 0) {
        // if it has children defined, then implement
        // TODO check there is no template type in the list
        if (node->ty.children) {
          // register children-types first
          ts_register_types(node->ty.children);

          // check that there is no template children
          bool templated = false;
          for (int i = 0; i < node->ty.children->list.length; ++i) {
            if (ty_is_templated(node->ty.children->list.values[i]->ty_id)) {
              templated = true;
            }
          }

          if (templated) {
            ast_raise_error(node, "type error, try to implement a template "
                                  "using another template");
          }
          // implement
          el = ast_implement_struct(node->ty.children, el, 0);
          // enjoy :)
        } else {
          // if not it just is just a reference, get the type and wait to
          // be implemented later
        }
      }
      return node->ty.id->ty_id = node->ty_id = el->ty_id;
    }
  }

  log_warning("delayed type '%s'", tcstr);
  return 0;
}

ast_action_t __trav_register_types(AST_CB_T_HEADER) {
  if (mode == AST_TRAV_LEAVE)
    return 0;

  if (node->ty_id)
    return AST_SEARCH_CONTINUE;

  switch (node->type) {
  case AST_DECL_TEMPLATE:
    node->tpl.id->ty_id = node->ty.id->ty_id = node->ty_id =
        ty_create_template(node);
    break;
  case AST_DECL_STRUCT:
    // TODO this need review, we don't want a gap in the type table...
    // if has templates need to be implemented before has a ty_id
    // if (node->structure.tpls == 0) {
    ts_register_types(node->structure.fields);
    node->ty.id->ty_id = node->ty_id = ty_create_struct(node);
    //}
    break;
  case AST_DECL_FUNCTION:
    // declare the function
    ts_register_types(node->func.params);
    ts_register_types(node->func.ret_type);
    node->ty.id->ty_id = node->ty_id = ty_create_fn(node);

    // add the virtual to the list in the type, only once
    if (node->func.type == AST_FUNC_PROPERTY && node->ts_passes == 0) {
      node->ts_passes = 1;
      ty_struct_add_virtual(node);
    }

    // add the operator to the list in the type, only once
    if (node->func.type == AST_FUNC_OPERATOR && node->ts_passes == 0) {
      node->ts_passes = 1;
      ty_struct_add_operator(node);
    }
    break;
  case AST_TYPE: {
    // check wrappers
    ast_t* p = node->parent;
    __ts_string_to_tyid(node);

    // sometimes the types are not fully built so wait for the second pass
    // if (!node->ty_id) return AST_SEARCH_CONTINUE;

    switch (p->type) {
    case AST_DECL_FUNCTION:
    case AST_TYPE:
      // just ignore
      break;
    case AST_DTOR_VAR:
      p->ty_id = node->ty_id;
      p->var.id->ty_id = node->ty_id;
      ty_create_var(p);
      break;

    case AST_PARAMETER:
      p->ty_id = node->ty_id;
      p->param.id->ty_id = node->ty_id;
      break;
    case AST_DECL_STRUCT_FIELD:
      p->ty_id = node->ty_id;
      p->field.id->ty_id = node->ty_id;
      break;
    case AST_DECL_STRUCT_ALIAS:
      // at this point, struct is no created, so we cannot determine
      // the real type. we will do it @ty_create_struct
      break;
    case AST_CAST:
      p->ty_id = node->ty_id;
      break;
    case AST_EXPR_TYPEOF:
    case AST_EXPR_SIZEOF:
      p->ty_id = TS_I64;
      break;
    case AST_LIST: // TODO this need review
      break;
    case AST_DELETE:
    case AST_NEW: {
      p->ty_id = node->ty_id;
    } break;
    default: { fl_assert(false); }
    }
  }
  default: {} // supress warning
  }

  return AST_SEARCH_CONTINUE;
}

// TODO performance, this should be deep-verse instead of traverse
// return error
ast_t* ts_register_types(ast_t* node) {
  ast_traverse(node, __trav_register_types, 0, 0, 0, 0);
  return node;
}
