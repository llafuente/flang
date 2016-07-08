#include "flang/common.h"
#include "flang/libast.h"
#include "flang/libparser.h"
#include "flang/typesystem.h"
#include "flang/codegen_c.h"
#include "flang/debug.h"
#include <stdio.h>

//#define CG_DEBUG
#define CG_OUTPUT fprintf
#define cg_debug printf

char* cg_text = 0;

char* buffer = 0;
char* buffer2 = 0;
array* cg_stack;
u64 max_level = 0;
int cg_indent = 0;

typedef struct {
  FILE* decls;
  FILE* types;
  FILE* functions;
  FILE* run;
} cg_files_t;

cg_files_t* cg_fds;

//
#define scan_string(str_var, format, ...) \
  snprintf(buffer, 1024, format, ##__VA_ARGS__); \
  str_var = st_newc(buffer, st_enc_utf8);

#define stack_append(format, ...) \
do {\
  string* str; \
  scan_string(str, format, ##__VA_ARGS__); \
  array_append(cg_stack, (void*) str); \
} while(false);

string* cg_type(u64 ty_id) {
  ty_t ty = ts_type_table[ty_id];
  // cached?
  if (ty.cg) {
    return ty.cg;
  }

  string* buffer = st_new(64, st_enc_utf8);

  switch (ty.of) {
  case FL_POINTER:
    st_append(&buffer, cg_type(ty.ptr.to));
    st_append_c(&buffer, "*");
    break;
  case FL_VECTOR:
    st_append(&buffer, cg_type(ty.vector.to));
    st_append_c(&buffer, "[]");
    break;
  case FL_STRUCT: {
    st_append(&buffer, ty.id);
  } break;
  case FL_FUNCTION: {
    st_append_c(&buffer, ty.id ? ty.id->value : "Anonymous");
  } break;
  default: {} // remove warning
  }
  assert(buffer->length != 0);
  return ty.cg = buffer;
}


string* st_dquote(const string* str) {
  st_uc_t ch;

  const char* data = str->value;
  string* out = st_new(str->capacity, str->encoding);
  string* buffer = st_new(5, str->encoding);

  st_append_char(&out, '"');

#define ISDIGIT(c) ((c >= (st_uc_t)'0') && (c <= (st_uc_t)'9'))
  st_size_t len = str->used;
  while (len-- > 0) {
    ch = *(st_uc_t*)(data++);
    if (ch < 127) {
      if (isprint(ch)) {
        if (ch == '\\') {
          st_append_char(&out, ch);
        }
        st_append_char(&out, ch);
        continue;
      } else if (ch == '\a') { /* \a -> audible bell */
        ch = (st_uc_t)'a';
      } else if (ch == '\b') { /* \b -> backspace */
        ch = (st_uc_t)'b';
      } else if (ch == '\f') { /* \f -> formfeed */
        ch = (st_uc_t)'f';
      } else if (ch == '\n') { /* \n -> newline */
        ch = (st_uc_t)'n';
      } else if (ch == '\r') { /* \r -> carriagereturn */
        ch = (st_uc_t)'r';
      } else if (ch == '\t') { /* \t -> horizontal tab */
        ch = (st_uc_t)'t';
      } else if (ch == '\v') { /* \v -> vertical tab */
        ch = (st_uc_t)'v';
      }
      st_append_char(&out, '\\');
      st_append_char(&out, ch);
      continue;
    }
    if (ISDIGIT(ch)) {
      sprintf(buffer->value, "\\%03d", ch);
      st_append(&out, buffer);
    } else {
      sprintf(buffer->value, "\\%d", ch);
      st_append(&out, buffer);
    }
  }

  st_append_char(&out, '"');

  st__zeronull(out->value, out->used, out->encoding);
  st_delete(&buffer);

  return out;
}


char* readable_operator(int operator) {
  if (operator < 127) {
    snprintf(buffer2, 1024, "%c", operator);
    return buffer2;
  }
  return "/*TODO!*/"; // nothing atm!
/*
TK_DOTDOTDOT
TK_DOTDOT
TK_PLUSPLUS
TK_MINUSMINUS
*/

}

void cg_dbg(ast_t* node, u64 level) {
#ifdef CG_DEBUG
  cg_debug("%*s", (int)level, " ");
  cg_debug("// ");
  ast_dump_one(node);
  cg_debug("\n");

  for (int i = 0; i < cg_stack->size; ++i) {
    cg_debug("// stack[%d] %s\n", i, ((string*)cg_stack->data[i])->value);
  }
#endif
}

ast_action_t __codegen_cb(ast_trav_mode_t mode, ast_t* node, ast_t* parent, u64 level,
          void* userdata_in, void* userdata_out) {


  assert(node != 0);
  cg_dbg(node, level);

  switch (node->type) {
  case AST_PROGRAM: {
    if (mode == AST_TRAV_LEAVE) {
      CG_OUTPUT(cg_fds->run, "int run()");
      while(cg_stack->size) {
        CG_OUTPUT(cg_fds->run, "%s\n", ((string*) array_pop(cg_stack))->value);
      }
  }
  } break;
  case AST_IMPORT:
  case AST_MODULE:
  case AST_LIST:
  case AST_TYPE:
  case AST_PARAMETER: // manually handled @AST_DECL_FUNCTION
  break;
  case AST_BLOCK:
    if (mode == AST_TRAV_ENTER) {
      cg_indent += 2;
      node->stack = cg_stack->size;
    } else {
      int buffer_size = 10 + cg_indent;

      for (int i = node->stack; i < cg_stack->size; ++i) {
        buffer_size += cg_indent + ((string*) cg_stack->data[i])->length + 2;
      }

      string* block = st_new(buffer_size, st_enc_utf8);
      st_append_c(&block, "{\n");

      for (int i = node->stack; i < cg_stack->size; ++i) {
        snprintf(buffer, 1024, "%*s%s;\n", cg_indent, " ",
          ((string*)cg_stack->data[i])->value);
        st_append_c(&block, buffer);
      }
      cg_stack->size = node->stack;

      cg_indent -= 2;
      snprintf(buffer, 1024, "%*s}\n", cg_indent, " ");
      st_append_c(&block, buffer);
      array_append(cg_stack, (void*) block);
    }
    break;
  case AST_EXPR_ASSIGNAMENT:
    if (mode == AST_TRAV_LEAVE) {
      string* right = (string*) array_pop(cg_stack);
      string* left = (string*) array_pop(cg_stack);

      assert(left != 0);
      assert(right != 0);

      stack_append("(%s = %s)", left->value, right->value);
    }
    break;

  case AST_EXPR_BINOP:
    if (mode == AST_TRAV_LEAVE) {
      string* right = (string*) array_pop(cg_stack);
      string* left = (string*) array_pop(cg_stack);

      assert(left != 0);
      assert(right != 0);

      stack_append("(%s %s %s)", left->value, readable_operator(node->binop.operator), right->value);
    }
    break;

  case AST_LIT_INTEGER: {
    if (mode == AST_TRAV_LEAVE) return 0;

    size_t ty = node->ty_id;
    // size_t ty = node->numeric.ty_id;
    ty_t t = ts_type_table[ty];

    // TODO REVIEW happens on cast, this should be promoted to AST_LIT_DECIMAL?
    //if (t.number.fp) {
    //  return LLVMConstReal(tref, node->integer.signed_value ? node->integer.signed_value : node->integer.unsigned_value);
    if (t.number.sign) {
      stack_append("%ld", node->integer.signed_value);
    } else {
      stack_append("%zu", node->integer.unsigned_value);
    }
  } break;

  case AST_LIT_FLOAT: {
    if (mode == AST_TRAV_LEAVE) return 0;
    if (parent->type == AST_EXPR_CALL) return AST_SEARCH_CONTINUE;

    stack_append("%f", node->decimal.value);
  } break;

  case AST_LIT_IDENTIFIER:
    if (mode == AST_TRAV_LEAVE) return 0;
    if (parent->type == AST_TYPE) return AST_SEARCH_CONTINUE;
    if (parent->type == AST_DECL_FUNCTION) return AST_SEARCH_CONTINUE;
    //if (parent->type == AST_PARAMETER) return AST_SEARCH_CONTINUE;
    //if (parent->type == AST_EXPR_CALL) return AST_SEARCH_CONTINUE;

    assert(node->identifier.string != 0);
    array_append(cg_stack, (void*) node->identifier.string);
    break;

  case AST_LIT_STRING:
    if (mode == AST_TRAV_LEAVE) return 0;

    // TODO add quotes
    array_append(cg_stack, (void*) st_dquote(node->string.value));
    break;

  case AST_LIT_BOOLEAN:
    if (mode == AST_TRAV_LEAVE) return 0;

    array_append(cg_stack, node->boolean.value ? st_newc("true", st_enc_utf8) : st_newc("false", st_enc_utf8));
    break;

  case AST_EXPR_LUNARY:
    if (mode == AST_TRAV_LEAVE) {
      string* right = (string*) array_pop(cg_stack);

      stack_append("(%s %s)", readable_operator(node->lunary.operator), right->value);
    }
    break;

  case AST_EXPR_RUNARY:
    if (mode == AST_TRAV_LEAVE) {
      string* left = (string*) array_pop(cg_stack);

      stack_append("(%s %s)", readable_operator(node->runary.operator), left->value);
    }
    break;
  case AST_EXPR_MEMBER:
    if (mode == AST_TRAV_LEAVE) {
      string* right = (string*) array_pop(cg_stack);
      string* left = (string*) array_pop(cg_stack);

      stack_append("(%s.%s)", left->value, right->value);
    }
    break;
  case AST_DTOR_VAR:
    if (mode == AST_TRAV_LEAVE) {
      string* id = (string*) array_pop(cg_stack);

      stack_append("%s %s", cg_type(node->ty_id)->value, id->value);
    }
    break;
  /*
  case AST_TYPE:
    break;
  */
  case AST_DECL_STRUCT:
    if (mode == AST_TRAV_ENTER) {
      node->stack = cg_stack->size;
    } else {
      int buffer_idx = 0;
      buffer2[0] = 0;
      string* id = (string*) cg_stack->data[node->stack];
      // TODO REVIEW skip the first one is the identifier atm
      for (int i = node->stack + 1; i < cg_stack->size; ++i) {
        string* arg = (string*) cg_stack->data[i];

        buffer_idx += snprintf(buffer2 + buffer_idx, 1024, "%s;\n", arg->value);
      }
      cg_stack->size = node->stack;

      CG_OUTPUT(cg_fds->decls, "struct %s__cg;\n", id->value);
      CG_OUTPUT(cg_fds->decls, "typedef struct %s__cg %s;\n", id->value, id->value);
      CG_OUTPUT(cg_fds->types, "struct %s__cg {\n%s\n};\n\n", id->value, buffer2);
    }
    break;
  case AST_DECL_STRUCT_FIELD:
    if (mode == AST_TRAV_LEAVE) {
      string* id = (string*) array_pop(cg_stack);
      stack_append("%s %s", cg_type(node->ty_id)->value, id->value);
    }
    break;
  case AST_DECL_FUNCTION:
    if (node->func.templated) {
      // templated function are not exported, it's clones are
      return AST_SEARCH_CONTINUE;
    }

    if (mode == AST_TRAV_ENTER) {
      u64 current = cg_stack->size;
      int buffer_idx = 0;
      buffer[0] = 0;

      if (node->func.params->list.count) {
        u64 i = 0;
        ast_t* tmp;
        ast_t** itr = node->func.params->list.elements;

        while ((tmp = itr[i++]) != 0) {
          ast_traverse(tmp, __codegen_cb, 0, 0, 0, 0);
          string* param = (string*) array_pop(cg_stack);

          //stack_append("%s %s", cg_type(tmp->ty_id)->value, param->value);
          if (itr[i] == 0) {
            buffer_idx += snprintf(buffer + buffer_idx, 1024, "%s %s", cg_type(tmp->ty_id)->value, param->value);
          } else {
            buffer_idx += snprintf(buffer + buffer_idx, 1024, "%s %s, ", cg_type(tmp->ty_id)->value, param->value);
          }
        }
      }

      if (node->func.varargs) {
        if (buffer_idx != 0) {
          buffer[buffer_idx++] = ',';
        }
        buffer[buffer_idx++] = ' ';
        buffer[buffer_idx++] = '.';
        buffer[buffer_idx++] = '.';
        buffer[buffer_idx++] = '.';
        buffer[buffer_idx++] = '\0';
      }

      string* parameters = st_newc(buffer, st_enc_utf8);

      if (!node->func.ffi) {
        // generate the block only, skip the rest
        ast_traverse(node->func.body, __codegen_cb, 0, 0, 0, 0);

        string* body = (string*) array_pop(cg_stack);

        CG_OUTPUT(cg_fds->decls, "%s %s (%s);\n",
          cg_type(node->func.ret_type->ty_id)->value,
          node->func.uid->value,
          parameters->value
        );

        CG_OUTPUT(cg_fds->functions, "%s %s (%s) %s",
          cg_type(node->func.ret_type->ty_id)->value,
          node->func.uid->value,
          parameters->value,
          body->value
        );
      } else {
        // TODO skip this processs atm
        return AST_SEARCH_SKIP;

        stack_append("extern %s %s (%s)",
          cg_type(node->func.ret_type->ty_id)->value,
          node->func.uid->value,
          parameters->value
        );
      }


      return AST_SEARCH_SKIP;
    }

    break;

    case AST_EXPR_CALL:

    if (mode == AST_TRAV_ENTER) {
      node->stack = cg_stack->size;
    } else {
      int buffer_idx = 0;
      buffer2[0] = 0;
      // TODO REVIEW skip the first one is the identifier atm
      for (int i = node->stack + 1; i < cg_stack->size; ++i) {
        string* arg = (string*) cg_stack->data[i];

        if (i == cg_stack->size - 1) {
          buffer_idx += snprintf(buffer2 + buffer_idx, 1024, "%s", arg->value);
        } else {
          buffer_idx += snprintf(buffer2 + buffer_idx, 1024, "%s, ", arg->value);
        }
      }
      cg_stack->size = node->stack;

      stack_append("%s(%s)", node->call.decl->func.uid->value, buffer2);
    }

    break;
  /*
  case AST_DECL_TEMPLATE:
    break;
  */
  case AST_STMT_RETURN:
    if (mode == AST_TRAV_LEAVE) {
      string* expr = (string*) array_pop(cg_stack);

      stack_append("return %s", expr->value);
    }
    break;
  /*
  case AST_ERROR:
    break;
  */
  case AST_STMT_COMMENT:
    if (mode == AST_TRAV_LEAVE) {
      stack_append("/* %s */", node->comment.text->value);
    }
    break;
  /*
  case AST_STMT_IF:
    break;
  case AST_STMT_LOOP:
    break;
  case AST_EXPR_SIZEOF:
    break;
  case AST_STMT_LOG:
    break;
  */
  case AST_CAST:
    if (mode == AST_TRAV_LEAVE) {
      string* expr = (string*) array_pop(cg_stack);
      stack_append("((%s) %s)", cg_type(node->ty_id)->value, expr->value);
    }

    break;
  /*
  case AST_ATTRIBUTE:
    break;
    */
  default: {
    printf("// node ignored?! %d\n", node->type);
  }
  }

  return AST_SEARCH_CONTINUE;
}


char* fl_codegen(ast_t* root) {
  //log_debug_level = 10;
  //ast_dump(root);
  //exit(0);



  cg_stack = calloc(sizeof(array), 1);
  buffer = calloc(sizeof(char), 1024);
  buffer2 = calloc(sizeof(char), 1024);
  //array_newcap(cg_stack, 500);
  array_newcap(cg_stack, 5);

  cg_fds = calloc(sizeof(cg_files_t), 1);
  cg_fds->decls = fopen("codegen/decls.c", "w");
  cg_fds->types = fopen("codegen/types.c", "w");
  cg_fds->functions = fopen("codegen/functions.c", "w");
  cg_fds->run = fopen("codegen/run.c", "w");

  CG_OUTPUT(cg_fds->run, "\n"
    "#include \"header.c\"\n"
    "#include \"decls.c\"\n"
    "#include \"types.c\"\n"
    "#include \"functions.c\"\n\n"
  );

  ast_traverse(root, __codegen_cb, 0, 0, 0, 0);

  fclose(cg_fds->decls);
  fclose(cg_fds->types);
  fclose(cg_fds->functions);
  fclose(cg_fds->run);

  free(cg_fds);
  free(cg_stack);
  free(buffer);
  free(buffer2);

  return 0;
}
