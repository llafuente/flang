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
#include "flang/libast.h"
#include "flang/libts.h"
#include "flang/debug.h"

ast_action_t __trav_replace_types(ast_trav_mode_t mode, ast_t* node,
                                  ast_t* parent, u64 level, void* userdata_in,
                                  void* userdata_out) {
  if (mode == AST_TRAV_LEAVE)
    return 0;

  if (node->ty_id == (*(u64*)userdata_in)) {
    node->ty_id = *((u64*)userdata_out);
    log_silly("type changed to %lu", node->ty_id);

    if (node->type == AST_TYPE) {
      node->ty.id->ty_id = node->ty_id;

      ast_t* p = node->parent;
      while (p) {
        if (p->type == AST_TYPE) {
          log_silly("parent is type: before %lu", p->ty_id);
          p->ty_id = 0;
          ts_register_types(p);
          log_silly("parent is type: after %lu", p->ty_id);
        }

        p = p->parent;
      }

      return AST_SEARCH_SKIP;
    }
  }

  return AST_SEARCH_CONTINUE;
}

void ast_replace_types(ast_t* node, u64 old, u64 new) {
  ast_traverse(node, __trav_replace_types, 0, 0, (void*)&old, (void*)&new);
}

void ast_clear(ast_t* node, ast_types_t type) {
  ast_t* p = node->parent;
  u32 first_line = node->first_line;
  u32 first_column = node->first_column;
  u32 last_line = node->last_line;
  u32 last_column = node->last_column;

  memset(node, 0, sizeof(ast_t));

  node->parent = p;
  node->first_line = first_line;
  node->first_column = first_column;
  node->last_line = last_line;
  node->last_column = last_column;

  node->type = type;
}
