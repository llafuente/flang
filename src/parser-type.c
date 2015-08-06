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

/*
literal
  []
  {}
  text

format
  (literal)[]      // array
  (literal){}      // object
  *(literal)       // raw pointer
  ref<(literal)>   // wrapper

  void - LLVMVoidTypeInContext(state.context)

*/
PSR_READ_IMPL(type) {
  log_verbose("start");
  PSR_START(type_node, FL_AST_TYPE);

  // primitives
  fl_tokens_t tk = state->token->type;
  fl_tokens_t tks[] = {FL_TK_VOID, FL_TK_BOOL,

                       FL_TK_I8,   FL_TK_U8,   FL_TK_I16,   FL_TK_U16,
                       FL_TK_I32,  FL_TK_U32,  FL_TK_I64,   FL_TK_U64,

                       FL_TK_F32,  FL_TK_F64,  FL_TK_STRING};

  size_t i;
  for (i = 0; i < 13; ++i) {
    if (tk == tks[i]) {
      type_node->ty_id = i + 1;
      PSR_NEXT();
      log_verbose("built-in");
      PSR_RET_OK(type_node);
    }
  }

  // primitive fail, try wrapper
  PSR_READ_OR_DIE(id, lit_identifier, { fl_ast_delete(type_node); }, 0);

  PSR_SKIPWS();

  // wapper type<subtype>
  if (PSR_ACCEPT_TOKEN(FL_TK_LT)) {
    PSR_SKIPWS();

    PSR_READ_OR_DIE(child, type, {
      fl_ast_delete(type_node);
      fl_ast_delete(id);
    }, "type expected");

    PSR_SKIPWS();

    PSR_EXPECT_TOKEN(FL_TK_GT, type_node, {
      fl_ast_delete(id);
      fl_ast_delete(child);
    }, "expected '>'");

    // unroll recursion, creating new types
    // handle "primitive" defined wrappers
    if (strcmp(id->identifier.string->value, "ptr") == 0) {
      type_node->ty_id = ts_wapper_typeid(FL_POINTER, child->ty_id);
    }

    if (strcmp(id->identifier.string->value, "vector") == 0) {
      type_node->ty_id = ts_wapper_typeid(FL_VECTOR, child->ty_id);
    }

    // TODO handle user defined wrappers
    // TODO handle builtin defined wrappers

    // void is a primitive will never reach here, it's safe to check != 0
    fl_ast_delete(child);
    fl_ast_delete(id);

    if (type_node->ty_id) {
      log_verbose("wrapped type");
      PSR_RET_OK(type_node);
    }
    // do something?!
  }

  size_t ty_id = ts_named_typeid(id->identifier.string);
  log_debug("**TYPE [%zu] = '%s'", ty_id, id->identifier.string->value);
  fl_ast_delete(id);

  if (!ty_id) {
    log_verbose("type not found");
    PSR_RET_SYNTAX_ERROR(type_node, "unkown type");
  }

  log_verbose("named type");
  type_node->ty_id = ty_id;
  PSR_RET_OK(type_node);
}

PSR_READ_IMPL(decl_struct) {
  if (!PSR_TEST_TOKEN(FL_TK_STRUCT)) {
    return 0;
  }

  PSR_START(structure, FL_AST_DECL_STRUCT);
  fl_type_t st_type;
  st_type.of = FL_STRUCT;

  PSR_ACCEPT_TOKEN(FL_TK_STRUCT);
  PSR_SKIPWS();

  PSR_READ_OR_DIE(id, lit_identifier_rw, { fl_ast_delete(structure); },
                  "expected identifier"); // no anonymous structs!
  PSR_SKIPWS();

  structure->structure.id = id;

  PSR_EXPECT_TOKEN(FL_TK_LCBRACKET, structure, {}, "expected '{'");
  PSR_SKIPWS();

  PSR_START_LIST(list);
  structure->structure.fields = list;

  if (!PSR_TEST_TOKEN(FL_TK_RCBRACKET)) {
    do {
      PSR_SKIPWS();

      PSR_START(field, FL_AST_DECL_STRUCT_FIELD);

      PSR_READ_OR_DIE(type, type, {
        fl_ast_delete(structure);
        fl_ast_delete(field);
      }, "expected type");
      PSR_SKIPWS();
      field->field.type = type;

      PSR_READ_OR_DIE(id_field, lit_identifier, {
        fl_ast_delete(structure);
        fl_ast_delete(field);
      }, "expected identifier");
      PSR_SKIPWS();
      field->field.id = id_field;

      PSR_END(field);

      list->list.elements[list->list.count++] = field;
    } while (PSR_ACCEPT_TOKEN(FL_TK_COMMA));
  }

  PSR_EXPECT_TOKEN(FL_TK_RCBRACKET, structure, {}, "expected '}'");

  structure->ty_id = ts_struct_create(structure);

  PSR_RET_OK(structure);
}

PSR_READ_IMPL(cast) {
  PSR_START(cast, FL_AST_CAST);

  if (!PSR_ACCEPT_TOKEN(FL_TK_CAST)) {
    fl_ast_delete(cast);
    cast = 0;

    PSR_RET_READED(cast, expr_conditional);

    return 0;
  }

  PSR_SKIPWS();
  PSR_EXPECT_TOKEN(FL_TK_LPARENTHESIS, cast, {}, "expected '('");

  PSR_SKIPWS();
  PSR_READ_OR_DIE(ty, type, { fl_ast_delete(cast); }, "expected expression");

  PSR_SKIPWS();
  PSR_EXPECT_TOKEN(FL_TK_RPARENTHESIS, cast, { fl_ast_delete(ty); },
                   "expected ')'");

  PSR_SKIPWS();
  PSR_READ_OR_DIE(element, expr_conditional, {
    fl_ast_delete(cast);
    fl_ast_delete(ty);
  }, "expected expression");

  cast->ty_id = ty->ty_id;
  fl_ast_delete(ty);
  cast->cast.element = element;

  PSR_RET_OK(cast);
}
