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
#include <limits.h>
#include <errno.h>

PSR_READ_IMPL(literal) {
  ast_t* ast;

  // null must be above, because if numeric found null
  // will be parsed as 0

  FL_TRY_READ(lit_null);
  FL_TRY_READ(lit_boolean);
  FL_TRY_READ(lit_string);
  FL_TRY_READ(lit_numeric);
  FL_TRY_READ(lit_identifier);

  return 0;
}

PSR_READ_IMPL(lit_null) {
  if (!PSR_TEST_TOKEN(FL_TK_NULL)) {
    return 0;
  }

  PSR_START(ast, FL_AST_LIT_NULL);

  PSR_NEXT(); // already "accepted" above

  PSR_RET_OK(ast);
}

PSR_READ_IMPL(lit_boolean) {
  PSR_START(ast, FL_AST_LIT_BOOLEAN);

  if (PSR_ACCEPT_TOKEN(FL_TK_TRUE)) {
    ast->boolean.value = true;
    PSR_RET_OK(ast);
  }
  if (PSR_ACCEPT_TOKEN(FL_TK_FALSE)) {
    ast->boolean.value = false;
    PSR_RET_OK(ast);
  }

  PSR_RET_KO(ast);
}

PSR_READ_IMPL(lit_string_sq) {
  if (!PSR_TEST_TOKEN(FL_TK_SQUOTE)) {
    return 0;
  }

  PSR_START(str_node, FL_AST_LIT_STRING);

  PSR_NEXT(); // already "accepted" above

  str_node->string.value = st_unescape(state->token->string);

  PSR_NEXT();

  if (!PSR_ACCEPT_TOKEN(FL_TK_SQUOTE)) {
    PSR_RET_KO(str_node);
  }

  PSR_RET_OK(str_node);
}

PSR_READ_IMPL(lit_string_dq) {
  if (!PSR_TEST_TOKEN(FL_TK_DQUOTE)) {
    return 0;
  }

  PSR_START(str_node, FL_AST_LIT_STRING);

  PSR_NEXT(); // already "accepted" above

  str_node->string.value = st_unescape(state->token->string);

  PSR_NEXT();

  if (!PSR_ACCEPT_TOKEN(FL_TK_DQUOTE)) {
    PSR_RET_KO(str_node);
  }

  str_node->ty_id = 15; // TODO ptr<i8> atm -> string in the future
  PSR_RET_OK(str_node);
}

PSR_READ_IMPL(lit_string) {
  ast_t* str_node;

  str_node = PSR_READ(lit_string_sq);
  if (str_node) {
    return str_node;
  }

  str_node = PSR_READ(lit_string_dq);
  if (str_node) {
    return str_node;
  }

  return 0;
}

// TODO
PSR_READ_IMPL(lit_array) { return 0; }
// TODO
PSR_READ_IMPL(lit_object) { return 0; }

// TODO manage overflow/underflow errors (syntax errors?)
PSR_READ_IMPL(lit_numeric) {
  PSR_START(ast, FL_AST_LIT_NUMERIC);

  // null is and alias of 0

  if (PSR_ACCEPT_TOKEN(FL_TK_NULL)) {
    ast->ty_id = 10; // u64
    PSR_NEXT();
    PSR_RET_OK(ast);
  }

  string* str = state->token->string;
  char* start = str->value;
  if (isdigit(start[0])) {
    bool integer = true;
    char* endp = start + (str->used);
    char* buffer = 0;

    // check next token if "."
    PSR_NEXT();
    if (PSR_TEST_TOKEN(FL_TK_DOT)) {
      PSR_NEXT();
      char* decimal = state->token->string->value;
      if (isdigit(start[0])) {
        // concat
        buffer = malloc(100); // TODO proper calculation
        buffer[0] = '\0';
        strcat(buffer, start);
        strcat(buffer, ".");
        strcat(buffer, decimal);
        start = buffer;
        endp = strchr(buffer, '\0');
        PSR_NEXT();
      }
    }

    char* p = start;
    while (p < endp) {
      // anything non number / '-', cannot be parsed by strod/l
      if (!isdigit(*p) && *p != '-') {
        integer = false;
        break;
      }
      ++p;
    }

    if (integer) {
      log_verbose("try to read long int");
      // try to parse a long int
      long val = strtol(start, &endp, 10);
      ast->numeric.ty_id = 9;

      log_verbose("read [%ld] [%d == %d]", val, errno, ERANGE);

      if (errno == ERANGE ||
          (errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)) ||
          (errno != 0 && val == 0)) {
        log_verbose("ERANGE: try to read long unsigned int");
        ast->numeric.li_value = 0;
        ast->numeric.lui_value = strtoul(start, &endp, 10);
        ast->numeric.ty_id = 10;
        if (errno == ERANGE) {
          log_verbose("ERANGE: double");
          ast->numeric.lui_value = 0;
        }
      } else {
        ast->numeric.li_value = val;
      }
    }

    // if integer fail... double!
    if (ast->numeric.li_value == 0 && ast->numeric.lui_value == 0) {
      log_verbose("double");

      // must be a number and error will be final!
      double result = strtod(start, &endp);
      if (errno) {
        if ((result == HUGE_VAL || result == -HUGE_VAL) && errno == ERANGE) {
          fprintf(stderr, "ERROR! overflow\n");
        } else if (errno == ERANGE) {
          fprintf(stderr, "ERROR! underflow\n");
        }
      }
      ast->numeric.d_value = result;

      // even if the initial parting fail, this could work...
      if ((double)((long int)result) == result) {
        log_verbose("long int");
        ast->numeric.ty_id = 9;
        ast->numeric.li_value = ast->numeric.d_value;
      } else if ((double)((long unsigned int)result) == result) {
        log_verbose("long unsigned int");
        ast->numeric.ty_id = 10;
        ast->numeric.lui_value = ast->numeric.d_value;
      } else {
        log_verbose("double");
        ast->numeric.ty_id = 12; // bigger possible f64
      }
    }
    ast->ty_id = ast->numeric.ty_id;

    if (buffer) {
      free(buffer);
    }

    PSR_RET_OK(ast);
  }

  PSR_RET_KO(ast);
}

// TODO review what should be valid and what not
// right now we should accept "anything that is not token"
// except string that is part of the core atm.
PSR_READ_IMPL(lit_identifier) {
  if (state->token->type != FL_TK_UNKOWN &&
      state->token->type != FL_TK_STRING) {
    return 0;
  }

  PSR_START(id_node, FL_AST_LIT_IDENTIFIER);

  id_node->identifier.string = st_clone(state->token->string);
  PSR_NEXT();

  PSR_RET_OK(id_node);
}

// any literal but punctuation
// use it with caution
PSR_READ_IMPL(lit_identifier_rw) {
  PSR_START(id_node, FL_AST_LIT_IDENTIFIER);

  id_node->identifier.string = st_clone(state->token->string);
  PSR_NEXT();

  PSR_RET_OK(id_node);
}
