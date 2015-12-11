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

// TODO global vars!
size_t ast_get_ident_typeid(ast_t* id) {
  assert(id->type == FL_AST_LIT_IDENTIFIER);
  log_verbose("%s", id->identifier.string->value);

  ast_t* decl = ast_search_id_decl(id, id->identifier.string);

  if (!decl) {
    log_error("(ts) cannot find var declaration %s",
              id->identifier.string->value);
  }

  return ast_get_typeid(decl);
}

ast_t* ast_get_attribute(ast_t* list, string* needle) {
  assert(list->type == FL_AST_LIST);
  ast_t* attr;
  size_t i;

  for (i = 0; i < list->list.count; ++i) {
    // exit when reach parent
    attr = list->list.elements[i];
    assert(attr->type == FL_AST_ATRRIBUTE);

    if (st_cmp(needle, attr->attr.id->identifier.string) == 0) {
      return attr;
    }
  }

  return 0;
}
