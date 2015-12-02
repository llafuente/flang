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

ast_action_t ast_is_literal_cb(ast_t* node, ast_t* parent, size_t level,
                              void* userdata_in, void* userdata_out) {
  switch (node->type) {
  case FL_AST_LIT_STRING:
  case FL_AST_LIT_FLOAT:
  case FL_AST_LIT_INTEGER:
    return FL_AC_CONTINUE;
  default: {} // supress warning
  }
  if (node->type == FL_AST_EXPR_LUNARY) {
    switch(node->lunary.operator) {
      case '-':
      case '!':
      case '&':
        return FL_AC_CONTINUE;
      default: {} // supress warning
    }
  }

  bool* ret = (bool*)userdata_out;
  *ret = false;
  return FL_AC_STOP;
}

bool ast_is_literal(ast_t* node) {
  bool b = true; // starts true, if find something not static -> false
  ast_traverse(node, ast_is_literal_cb, 0, 0, 0, (void*)&b);
  return b;
}
