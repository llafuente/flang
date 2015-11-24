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

// only promote numbers
size_t ts_promote_typeid(size_t a, size_t b) {
  ty_t t_a = ts_type_table[a];
  ty_t t_b = ts_type_table[b];

  assert(t_a.of == FL_NUMBER);
  assert(t_b.of == FL_NUMBER);

  // check floating point
  if (t_a.number.fp && !t_b.number.fp) {
    return a;
  }

  if (t_b.number.fp && !t_a.number.fp) {
    return b;
  }
  // check different sign
  if (!t_b.number.sign && t_a.number.sign) {
    return a;
  }

  if (t_b.number.sign && !t_a.number.sign) {
    return b;
  }

  // check bits
  return t_a.number.bits > t_b.number.bits ? a : b;
}

// TODO global vars!
size_t ts_var_typeid(ast_t* id) {
  assert(id->type == FL_AST_LIT_IDENTIFIER);
  log_verbose("%s", id->identifier.string->value);

  ast_t* decl = ast_search_id_decl(id, id->identifier.string);

  if (!decl) {
    log_error("(ts) cannot find var declaration %s",
              id->identifier.string->value);
  }

  return ast_get_typeid(decl);
}

ts_typeh_t* ts_named_type(string* id) {
  ts_typeh_t* s;
  HASH_FIND_STR(ts_hashtable, id->value, s);

  return s;
}
