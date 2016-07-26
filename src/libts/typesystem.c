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

#include "flang/common.h"
#include "flang/flang.h"
#include "flang/libts.h"
#include "flang/libast.h"
#include "flang/libparser.h"
#include <setjmp.h>

static int ts_pending = 0;

ast_t* _typesystem(ast_t* root) {
  psr_ast_imports(root);

  ts_register_types(root);
  ts_implement(root);
  ts_register_types(root); // it's not redundant

  // do inference
  root = ts_pass(root);

  // reduce ast to it's minimal form
  root = ast_reduce(root);
}

ast_action_t __trav_raise_no_type(ast_trav_mode_t mode, ast_t* node,
                                  ast_t* parent, u64 level, void* userdata_in,
                                  void* userdata_out) {
  // these don't have a type
  switch (node->type) {
  case AST_DECL_FUNCTION:
    if (node->func.templated) {
      // functions with templates are not generated. typesystem ignore them
      // until they are implemented by expr-call or implement stmt
      return AST_SEARCH_SKIP;
    }
    return AST_SEARCH_CONTINUE;
    break;
  case AST_IMPLEMENT:
    return AST_SEARCH_SKIP;
  case AST_BLOCK:
  case AST_PROGRAM:
  case AST_MODULE:
  case AST_LIST:
  case AST_STMT_COMMENT:
  case AST_IMPORT:
  case AST_STMT_IF:   // REVIEW this may be and expr in the future
  case AST_STMT_LOOP: // REVIEW this may be and expr in the future
    return AST_SEARCH_CONTINUE;
  default: {} // remove warning
  }

  if (!node->ty_id) {
    ast_dump_s(node->parent);
    ast_dump_s(node);
    // ast_dump_s(node->parent->structure.from_tpl);
    ast_raise_error(node, "Cannot determine type");
    return AST_SEARCH_STOP;
  }

  return AST_SEARCH_CONTINUE;
}

ast_t* typesystem(ast_t* root) {
  ts_pending = 1;

  while (ts_pending && !ast_last_error_message) {
    if (!setjmp(fl_on_error_jmp)) {
      printf("setjmp!!\n");
      _typesystem(root);
    }
    ts_pending = 0; // just exit atm.
  }

  // now everything must have a type != 0
  // assert otherwise
  ast_traverse(root, __trav_raise_no_type, 0, 0, 0, 0);

  return root;
}
