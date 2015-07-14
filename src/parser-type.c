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
  PSR_AST_START(FL_AST_TYPE);

  // primitives
  fl_tokens_t tk = state->token->type;
  fl_tokens_t tks[] = {FL_TK_VOID, FL_TK_BOOL,

                       FL_TK_I8,   FL_TK_U8,   FL_TK_I16, FL_TK_U16,
                       FL_TK_I32,  FL_TK_U32,  FL_TK_I64, FL_TK_U64,

                       FL_TK_F32,  FL_TK_F64};

  size_t i;
  for (i = 0; i < 12; ++i) {
    if (tk == tks[i]) {
      ast->ty.id = i + 1;
      PSR_NEXT();

      PSR_AST_RET();
    }
  }

  // primitive fail, try wrapper
  fl_ast_t* id = PSR_READ(lit_identifier);
  if (!id) {
    PSR_AST_RET_NULL();
  }

  fl_parser_skipws(tokens, state);

  if (PSR_ACCEPT_TOKEN(FL_TK_LT)) {
    fl_parser_skipws(tokens, state);

    fl_ast_t* child = PSR_READ(type);
    if (child->type == FL_AST_ERROR) {
      fl_ast_delete(id);
      return child;
    }

    fl_parser_skipws(tokens, state);

    if (!PSR_ACCEPT_TOKEN(FL_TK_GT)) {
      // hard
      fl_ast_delete(id);
      fl_ast_delete(child);
      PSR_SYNTAX_ERROR(ast, "expected '>'");
    }

    // unroll recursion, creating new types
    // handle "primitive" defined wrappers
    if (strcmp(id->identifier.string->value, "ptr") == 0) {
      ast->ty.id = fl_parser_get_typeid(FL_POINTER, child->ty.id);
    }

    if (strcmp(id->identifier.string->value, "vector") == 0) {
      ast->ty.id = fl_parser_get_typeid(FL_VECTOR, child->ty.id);
    }

    // TODO handle user defined wrappers
    // TODO handle builtin defined wrappers

    // void is a primitive will never reach here, it's safe to check != 0
    fl_ast_delete(child);

    if (ast->ty.id) {
      fl_ast_delete(id);
      PSR_AST_RET();
    }
    // do something?!
  }

  fl_ast_delete(id);
  // TODO handle this could be user defined type

  PSR_AST_RET_NULL();
}

PSR_READ_IMPL(cast) {
  PSR_AST_START(FL_AST_CAST);

  if (!PSR_ACCEPT_TOKEN(FL_TK_CAST)) {
    fl_ast_delete(ast);
    ast = 0;

    FL_TRY_READ(expr_conditional);

    return 0;
  }

  // hard
  fl_parser_skipws(tokens, state);

  if (!PSR_ACCEPT_TOKEN(FL_TK_LPARENTHESIS)) {
    PSR_SYNTAX_ERROR(ast, "expected '('");
  }

  fl_ast_t* ty = PSR_READ(type);
  if (ty->type == FL_AST_ERROR) {
    fl_ast_delete(ast);
    return ty;
  }

  fl_parser_skipws(tokens, state);
  if (!PSR_ACCEPT_TOKEN(FL_TK_RPARENTHESIS)) {
    fl_ast_delete(ty);
    PSR_SYNTAX_ERROR(ast, "expected ')'");
  }

  fl_ast_t* right = PSR_READ(expr_conditional);

  ast->cast.to = ty;
  ast->cast.right = right;

  PSR_AST_RET();
}

fl_type_t* fl_type_table = 0;
size_t fl_type_size = 0;

void fl_parser_init_types() {
  if (!fl_type_table) {
    printf("\n\n\n*****INIT TABLE****\n\n\n");
    fl_type_table = calloc(sizeof(fl_type_t), 100);

    // 0 means infer!
    fl_type_table[0].of = FL_INFER;
    // type starts with 1
    fl_type_table[1].of = FL_VOID;

    size_t id = 2;
    // [1] bool
    fl_type_table[id].of = FL_NUMBER;
    fl_type_table[id].number.bits = 1;
    fl_type_table[id].number.fp = false;
    fl_type_table[id].number.sign = false;
    // [1-8] i8,u8,i16,u16,i32,u32,i64,u64
    size_t i = 3;
    for (; i < 7; i++) {
      fl_type_table[++id].of = FL_NUMBER;
      fl_type_table[id].number.bits = pow(2, i);
      fl_type_table[id].number.fp = false;
      fl_type_table[id].number.sign = false;

      fl_type_table[++id].of = FL_NUMBER;
      fl_type_table[id].number.bits = pow(2, i);
      fl_type_table[id].number.fp = false;
      fl_type_table[id].number.sign = true;
    }

    // [9] f32
    fl_type_table[++id].of = FL_NUMBER;
    fl_type_table[id].number.bits = 32;
    fl_type_table[id].number.fp = true;
    fl_type_table[id].number.sign = true;

    // [10] f64
    fl_type_table[++id].of = FL_NUMBER;
    fl_type_table[id].number.bits = 64;
    fl_type_table[id].number.fp = true;
    fl_type_table[id].number.sign = true;

    fl_type_size = ++id;
  }
}

size_t fl_parser_get_typeid(fl_types_t wrapper, size_t child) {
  size_t i;

  for (i = 0; i < fl_type_size; ++i) {
    if (fl_type_table[i].of == wrapper && fl_type_table[i].ptr.to == child) {
      return i;
    }
  }
  // add it!
  i = fl_type_size++;
  switch (wrapper) {
  case FL_POINTER:
    fl_type_table[i].of = wrapper;
    fl_type_table[i].ptr.to = child;
    break;
  case FL_VECTOR:
    fl_type_table[i].of = wrapper;
    fl_type_table[i].vector.size = 0;
    fl_type_table[i].vector.to = child;
    break;
  default: { cg_error("(parser) fl_parser_get_typeid fail\n"); }
  }

  return i;
}
