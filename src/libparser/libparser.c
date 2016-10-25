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
#include "flang/debug.h"
#include "flang/libparser.h"
#include <string.h>
#include <setjmp.h>

void psr_attach_core(ast_t* root) {
  ast_t* block = root->program.body;

  if (block->type != AST_ERROR) {
    ast_mk_list_insert(
        block->block.body,
        ast_mk_import(ast_mk_lit_string("lib/core/core", false), true), 0);
    ast_mk_list_insert(
        block->block.body,
        ast_mk_import(ast_mk_lit_string("lib/core/ffi-c", false), true), 0);
    ast_mk_list_insert(
        block->block.body,
        ast_mk_import(ast_mk_lit_string("lib/core/array", false), true), 0);
    ast_mk_list_insert(
        block->block.body,
        ast_mk_import(ast_mk_lit_string("lib/core/string", false), true), 0);
  }
}

ast_t* __psr_parse(string* code, const char* file) {
  // create program node, so error reporting could be nice!
  ast_t* root = ast_mk_program(0);
  root->program.code = code;
  root->program.file =
      file ? st_newc(file, st_enc_utf8) : st_newc("memory:string", st_enc_utf8);

  YY_BUFFER_STATE buf = yy_scan_string(code->value);
  yyparse(&root);
  yy_delete_buffer(buf);

  return root;
}

ast_t* psr_str_utf8(char* str) {
  st_size_t cap;

  string* code = st_new(cap + 2, st_enc_utf8);
  st_copyc(&code, str, st_enc_utf8);
  st_append_char(&code, 0);

  ast_t* root = __psr_parse(code, 0);
  ast_parent(root);

  return root;
}

ast_t* psr_str_utf8_main(char* str) {
  ast_t* root = psr_str_utf8(str);
  psr_attach_core(root);
  ast_parent(root);

  return root;
}

ast_t* psr_file(const char* filename) {
  string* code = psr_file_to_string(filename);
  ast_t* root = __psr_parse(code, filename);
  ast_parent(root);

  return root;
}

string* psr_file_to_string(const char* filename) {
  FILE* f = fopen(filename, "r");
  if (!f) {
    fl_fatal_error("file cannot be opened: %s\n", filename);
  }

  fseek(f, 0, SEEK_END);
  size_t lSize = ftell(f);
  rewind(f);

  string* code = st_new(lSize + 2, st_enc_utf8);
  // copy the file into the buffer:
  size_t result = fread(code->value, 1, lSize, f);
  if (result != lSize) {
    fl_fatal_error("%s\n", "Reading error");
  }

  // double null needed by flex/bison
  code->value[lSize] = '\0';
  code->value[lSize + 1] = '\0';

  code->used = result;
  code->length = st_utf8_length(code->value, 0);

  return code;
}

ast_t* psr_file_main(const char* filename) {
  string* code = psr_file_to_string(filename);

  ast_t* root = __psr_parse(code, filename);

  fl_assert(root->type == AST_PROGRAM);
  psr_attach_core(root);
  ast_parent(root);

  return root;
}

ast_action_t __trav_load_imports(AST_CB_T_HEADER) {
  if (mode == AST_TRAV_LEAVE)
    return 0;

  if (node->type == AST_IMPORT && !node->import.imported) {
    fl_assert(parent->type == AST_LIST);

    char* file = node->import.path->string.value->value;
    log_verbose("loading module %s", file);

    char filepath[1024] = "";

    if (file[0] == '.' && file[1] == '/') {
      ast_t* root = ast_get_root(node);

      strcat(filepath, dirname(root->program.file->value));
      strcat(filepath, "/");
      strcat(filepath, file + 2);
      strcat(filepath, ".fl");
    } else {
      strcat(filepath, "./");
      strcat(filepath, file);
      strcat(filepath, ".fl");
    }

    // printf("load module %s\n", filepath);

    ast_t* module = psr_file(filepath);

    if (ast_print_error(module)) {
      fl_fatal_error("parse error, failed to load module: '%s' at '%s'\n", file,
                     filepath);
    }

    module->type = AST_MODULE;

    // ast_mk_insert_before(parent, node, module);

    ast_t* block_scope = ast_get_scope(node);
    array_push(&block_scope->block.modules, module);
    module->parent = block_scope;

    log_verbose("module loaded %s at %p count=%lu", file, module,
                block_scope->block.modules.length);
    // no reimport
    node->import.imported = true;

    ((*(u64*)userdata_out))++;
  }

  return AST_SEARCH_CONTINUE;
}

// return error
ast_t* psr_ast_imports(ast_t* node) {
  u64 imported;
  do {
    imported = 0;
    ast_traverse(node, __trav_load_imports, 0, 0, 0, (void*)&imported);
  } while (imported);

  return 0;
}

char psr_buffer[10] = "";
char* psr_operator_str(int operator) {
  if (operator<127) {
    snprintf((char*)psr_buffer, 1024, "%c", operator);
    return (char*)psr_buffer;
  }

  switch (operator) {
  case TK_DOTDOTDOT:
    return "...";
  case TK_DOTDOT:
    return "..";
  case TK_EQEQ:
    return "==";
  case TK_FAT_ARROW:
    return "=>";
  case TK_NE:
    return "!=";
  case TK_LE:
    return "<=";
  case TK_SHL:
    return "<<";
  case TK_SHLEQ:
    return "<<=";
  case TK_GE:
    return ">=";
  case TK_SHR:
    return ">>";
  case TK_SHREQ:
    return ">>=";
  case TK_RARROW:
    return "->";
  case TK_MINUSMINUS:
    return "--";
  case TK_MINUSEQ:
    return "-=";
  case TK_ANDAND:
    return "&&";
  case TK_ANDEQ:
    return "&=";
  case TK_OROR:
    return "||";
  case TK_OREQ:
    return "|=";
  case TK_PLUSPLUS:
    return "++";
  case TK_PLUSEQ:
    return "+=";
  case TK_STAREQ:
    return "*=";
  case TK_SLASHEQ:
    return "/=";
  case TK_CARETEQ:
    return "^=";
  case TK_PERCENTEQ:
    return "%=";
  case TK_ACCESS:
    return "[]";
  case TK_ACCESS_MOD:
    return "[]=";
  }

  fl_fatal_error("%s: %d", "unkown operator found", operator);
  return 0;
}

ast_action_t __trav_psr_ast_check(AST_CB_T_HEADER) {
  if (mode == AST_TRAV_LEAVE) {
    return 0;
  }
  switch (node->type) {
  case AST_COMPILER_ERROR:
    ast_raise_error(node, "compiler error found");
  case AST_STMT_RETURN: {
    ast_t* block = ast_get_function_scope(node);
    if (!block) {
      ast_raise_error(node,
                      "syntax error, return found outside function scope");
    }
  } break;
  default: {} // remove warning
  }

  return AST_SEARCH_CONTINUE;
}

ast_t* psr_ast_check(ast_t* root) {
  if (!setjmp(fl_on_error_jmp)) {
    ast_traverse(root, __trav_psr_ast_check, 0, 0, 0, 0);
  }

  return root;
}
