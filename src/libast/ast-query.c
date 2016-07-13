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
#include "flang/libast.h"
#include "flang/libts.h"
#include "flang/debug.h"

//
// is
//

ast_action_t __trav_is_literal(ast_trav_mode_t mode, ast_t* node, ast_t* parent, u64 level,
                               void* userdata_in, void* userdata_out) {
  if (mode == AST_TRAV_LEAVE) return 0;

  switch (node->type) {
  case AST_LIT_STRING:
  case AST_LIT_FLOAT:
  case AST_LIT_INTEGER:
  case AST_EXPR_BINOP:
    return AST_SEARCH_CONTINUE;
  case AST_EXPR_LUNARY:
    switch (node->lunary.operator) {
    case '-':
    case '!':
    case '&':
      return AST_SEARCH_CONTINUE;
    default: {} // supress warning
    }
  default: {} // supress warning
  }

  bool* ret = (bool*)userdata_out;
  *ret = false;
  return AST_SEARCH_STOP;
}

bool ast_is_literal(ast_t* node) {
  bool b = true; // starts true, if find something not static -> false
  ast_traverse(node, __trav_is_literal, 0, 0, 0, (void*)&b);
  return b;
}

bool ast_is_pointer(ast_t* node) {
  u64 id = ast_get_typeid(node);
  return ts_type_table[id].of == FL_POINTER;
}

//
// get
//

// TODO UTF-8 support
string* ast_get_code(ast_t* node) {
  ast_t* root = ast_get_root(node);
  u64 max = root->program.code->used;
  char* start = root->program.code->value;
  char* end = 0;
  u64 line = 1;
  u64 column = 1;
  u64 i = 0;

  while ((node->first_line != line || node->first_column != column) &&
         i < max) {
    if (*start == '\n') {
      ++line;
      column = 0;
    }
    ++column;
    ++start;

    ++i;
  }

  end = start;

  while ((node->last_line != line || node->last_column != column) && i < max) {
    if (*end == '\n') {
      ++line;
      column = 0;
    }
    ++column;
    ++end;

    ++i;
  }

  string* ret = st_new_subc(start, end - start + 1, st_enc_utf8);

  return ret;
}

ast_t* ast_get_root(ast_t* node) {
  ast_t* root = node;
  if (root->type == AST_PROGRAM || root->type == AST_MODULE) {
    return root;
  }

  while (root) {
    root = root->parent;
    if (root->type == AST_PROGRAM || root->type == AST_MODULE) {
      return root;
    }
  }

  return 0;
}

char ast_get_location_buffer[1024];
string* ast_get_location(ast_t* node) {
  ast_t* root = ast_get_root(node);
  sprintf(ast_get_location_buffer, "%s:%d:%d",
          root->program.file->value,
          node->first_line, node->first_column);

  string* ret = st_newc(ast_get_location_buffer, st_enc_utf8);
  return ret;
}

ast_t* ast_get_scope(ast_t* node) {
  ast_t* blk = node;
  while (blk) {
    blk = blk->parent;
    assert(blk != 0);
    if (blk->type == AST_BLOCK &&
        blk->block.scope != AST_SCOPE_TRANSPARENT) {
      return blk;
    }
  }

  return 0;
}

ast_t* ast_get_global_scope(ast_t* node) {
  ast_t* blk = node;
  while (blk) {
    blk = blk->parent;
    if (blk->type == AST_BLOCK && blk->block.scope == AST_SCOPE_GLOBAL) {
      return blk;
    }
  }

  return 0;
}

ast_t* ast_get_attribute(ast_t* list, string* needle) {
  assert(list->type == AST_LIST);
  ast_t* attr;
  u64 i;

  for (i = 0; i < list->list.count; ++i) {
    // exit when reach parent
    attr = list->list.elements[i];
    assert(attr->type == AST_ATTRIBUTE);

    if (st_cmp(needle, attr->attr.id->identifier.string) == 0) {
      return attr;
    }
  }

  return 0;
}

u64 ast_get_typeid(ast_t* node) {
  assert(node != 0);
  // check AST is somewhat "type-related"
  switch (node->type) {
  case AST_DTOR_VAR:
    log_verbose("var decl '%s' T(%zu)", node->var.id->identifier.string->value,
                node->var.type->ty_id);
    return node->var.type->ty_id;
  case AST_PARAMETER:
    log_verbose("parameter T(%zu)", node->param.id->ty_id);
    return node->param.id->ty_id;
  case AST_EXPR_CALL: {
    // search function
  }
  case AST_EXPR_MEMBER:
  case AST_EXPR_BINOP: {
    // this is valid only after ts_pass
    return node->ty_id;
  }
  case AST_EXPR_ASSIGNAMENT: {
    return ast_get_typeid(node->assignament.left);
  }
  case AST_LIT_IDENTIFIER: {
    if (node->ty_id) {
      return node->ty_id;
    }

    log_verbose("identifier T(%s)", node->identifier.string->value);
    // search var-dtor and return it
    ast_t* dtor = ast_search_id_decl(node, node->identifier.string);
    if (dtor) {
      return ast_get_typeid(dtor);
    }
  } break;
  case AST_TYPE:
    log_verbose("type T(%zu)", node->ty_id);
    return node->ty_id;
  case AST_CAST:
    return node->ty_id;
  case AST_LIT_BOOLEAN:
    return 2; // TODO maybe 3, i8
  case AST_LIT_FLOAT:
  case AST_LIT_INTEGER:
    return node->ty_id;
  // dont give information, continue up
  case AST_EXPR_LUNARY:
    return ast_get_typeid(node->parent);
  case AST_LIT_STRING:
    return TS_STRING;
  case AST_DECL_FUNCTION:
    return node->ty_id;
  default: {}
  }
  log_error("ast_get_typeid: node is not type related! %d", node->type);
  return 0;
}

// TODO this can be removed... not used
u64 ast_get_struct_prop_idx(ast_t* decl, string* id) {
  u64 i;
  ast_t* list = decl->structure.fields;
  ast_t** elements = list->list.elements;
  u64 length = list->list.count;

  for (i = 0; i < length; ++i) {
    if (st_cmp(elements[i]->field.id->identifier.string, id)) {
      return i;
    }
  }
  return -1;
}
