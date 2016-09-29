#include "flang/flang.h"
#include "flang/libast.h"
#include "flang/libparser.h"
#include "flang/libts.h"
#include "flang/libcg.h"
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

string* cg_node(ast_t* node);

//
#define scan_string(str_var, format, ...)                                      \
  snprintf(buffer, 1024, format, ##__VA_ARGS__);                               \
  str_var = st_newc(buffer, st_enc_utf8);

#define stack_append(format, ...)                                              \
  do {                                                                         \
    string* str;                                                               \
    scan_string(str, format, ##__VA_ARGS__);                                   \
    array_push(cg_stack, (void*)str);                                          \
  } while (false);

string* cg_type(u64 ty_id) {
  ty_t type = ty(ty_id);
  // cached?
  if (type.cg != 0) {
    return type.cg;
  }

  string* buffer = st_new(64, st_enc_utf8);

  switch (type.of) {
  case TY_POINTER:
  case TY_REFERENCE:
    st_append(&buffer, cg_type(type.ptr.to));
    st_append_c(&buffer, "*");
    break;
  case TY_VECTOR:
    st_append(&buffer, cg_type(type.vector.to));
    st_append_c(&buffer, "[]");
    break;
  case TY_STRUCT: {
    st_append(&buffer, type.id);
  } break;
  case TY_FUNCTION: {
    fl_assert(type.id != 0);

    st_append_c(&buffer, type.id->value);
    st_append_c(&buffer, "__fnptr");
  } break;
  case TY_TEMPLATE: {
    // REVIEW assert... will see in the future if needed, i consider this
    // dangerous atm.
    // st_append_c(&buffer, "0");
    return type.id;
  } break;
  default: {
    printf("type.of %d\n", type.of);
    fl_assert(false);
  } // remove warning
  }

  fl_assert(buffer->length != 0);
  return type.cg = buffer;
}

string* st_dquote(const string* str) {
  st_uc_t ch;
  char* rep;

  const char* data = str->value;
  string* out = st_new(str->capacity + 2, str->encoding);
  string* buffer = st_new(5, str->encoding);

  st_append_char(&out, '"');

#define ISDIGIT(c) ((c >= (st_uc_t)'0') && (c <= (st_uc_t)'9'))
  st_size_t len = str->used;
  while (len-- > 0) {
    ch = *(st_uc_t*)(data++);

    if (ch < 127) {
      rep = 0;
      if (isprint(ch)) {
        if (ch == '"') {
          st_append_c(&out, "\\\"");
          continue;
        }

        if (ch == '\\') {
          st_append_char(&out, ch);
        }
        st_append_char(&out, ch);
        continue;
      }

      switch (ch) {
      case '\a': // \a -> audible bell
        rep = "\\a";
        break;
      case '\b': // \b -> backspace
        rep = "\\b";
        break;
      case '\f': // \f -> formfeed
        rep = "\\f";
        break;
      case '\n': // \n -> newline
        rep = "\\n";
        break;
      case '\r': // \r -> carriagereturn
        rep = "\\r";
        break;
      case '\t': // \t -> horizontal tab
        rep = "\\t";
        break;
      case '\v': // \v -> vertical tab
        rep = "\\v";
        break;
      }

      if (rep) {
        st_append_c(&out, rep);
      } else {
        // UTF-8? passthrough
        st_append_char(&out, ch);
      }
    } else if (ISDIGIT(ch)) {
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

void cg_dbg(ast_t* node, u64 level) {
#ifdef CG_DEBUG
  cg_debug("%*s", (int)level, " ");
  cg_debug("// ");
  ast_dump_one(node);
  cg_debug("\n");

  for (int i = 0; i < cg_stack->length; ++i) {
    cg_debug("// stack[%d] %s\n", i, ((string*)cg_stack->values[i])->value);
  }
#endif
}

ast_action_t __codegen_cb(ast_trav_mode_t mode, ast_t* node, ast_t* parent,
                          u64 level, void* userdata_in, void* userdata_out) {

  fl_assert(node != 0);
  cg_dbg(node, level);

  switch (node->type) {
  case AST_PROGRAM: {
    if (mode == AST_TRAV_LEAVE) {
      CG_OUTPUT(cg_fds->run, "void run()");
      while (cg_stack->length) {
        CG_OUTPUT(cg_fds->run, "%s\n", ((string*)array_pop(cg_stack))->value);
      }
    }
  } break;
  case AST_IMPORT:
  case AST_MODULE:
  case AST_LIST:
  case AST_TYPE:
  case AST_PARAMETER: // manually handled @AST_DECL_FUNCTION
    break;
  case AST_IMPLEMENT:         // handled @typesystem
  case AST_DECL_STRUCT_ALIAS: // not needed
    return AST_SEARCH_SKIP;
  case AST_BLOCK:
    if (mode == AST_TRAV_ENTER) {
      cg_indent += 2;
      node->stack = cg_stack->length;
    } else {
      int buffer_size = 10 + cg_indent;

      for (int i = node->stack; i < cg_stack->length; ++i) {
        buffer_size += cg_indent + ((string*)cg_stack->values[i])->length + 2;
      }

      string* block = st_new(buffer_size, st_enc_utf8);
      st_append_c(&block, "{\n");

      for (int i = node->stack; i < cg_stack->length; ++i) {
        snprintf(buffer, 1024, "%*s%s;\n", cg_indent, " ",
                 ((string*)cg_stack->values[i])->value);
        st_append_c(&block, buffer);
      }
      cg_stack->length = node->stack;

      cg_indent -= 2;
      snprintf(buffer, 1024, "%*s}\n", cg_indent, " ");
      st_append_c(&block, buffer);
      array_push(cg_stack, (void*)block);
    }
    break;
  case AST_EXPR_ASSIGNAMENT:
    if (mode == AST_TRAV_LEAVE) {
      string* right = (string*)array_pop(cg_stack);
      string* left = (string*)array_pop(cg_stack);

      fl_assert(left != 0);
      fl_assert(right != 0);

      stack_append("(%s = %s)", left->value, right->value);
    }
    break;

  case AST_EXPR_BINOP:
    if (mode == AST_TRAV_LEAVE) {
      string* right = (string*)array_pop(cg_stack);
      string* left = (string*)array_pop(cg_stack);

      fl_assert(left != 0);
      fl_assert(right != 0);

      stack_append("(%s %s %s)", left->value,
                   psr_operator_str(node->binop.operator), right->value);
    }
    break;

  case AST_LIT_INTEGER: {
    if (mode == AST_TRAV_LEAVE)
      return 0;

    // size_t ty = node->numeric.ty_id;
    ty_t type = ty(node->ty_id);

    // TODO REVIEW happens on cast, this should be promoted to AST_LIT_DECIMAL?
    // if (type.number.fp) {
    //  return LLVMConstReal(tref, node->integer.signed_value ?
    //  node->integer.signed_value : node->integer.unsigned_value);
    if (type.number.sign) {
      stack_append("%ld", node->integer.signed_value);
    } else {
      stack_append("%zu", node->integer.unsigned_value);
    }
  } break;

  case AST_LIT_FLOAT: {
    if (mode == AST_TRAV_LEAVE)
      return 0;
    if (parent->type == AST_EXPR_CALL)
      return AST_SEARCH_CONTINUE;

    stack_append("%f", node->decimal.value);
  } break;

  case AST_LIT_IDENTIFIER:
    if (mode == AST_TRAV_LEAVE)
      return 0;
    if (parent->type == AST_TYPE)
      return AST_SEARCH_CONTINUE;
    if (parent->type == AST_DECL_FUNCTION)
      return AST_SEARCH_CONTINUE;
    // if (parent->type == AST_PARAMETER) return AST_SEARCH_CONTINUE;
    // if (parent->type == AST_EXPR_CALL) return AST_SEARCH_CONTINUE;

    fl_assert(node->identifier.string != 0);
    array_push(cg_stack, (void*)node->identifier.string);
    break;

  case AST_LIT_STRING:
    if (mode == AST_TRAV_LEAVE)
      return 0;

    array_push(cg_stack, (void*)st_dquote(node->string.value));
    break;

  case AST_LIT_BOOLEAN:
    if (mode == AST_TRAV_LEAVE)
      return 0;

    array_push(cg_stack, node->boolean.value ? st_newc("true", st_enc_utf8)
                                             : st_newc("false", st_enc_utf8));
    break;

  case AST_EXPR_LUNARY:
    if (mode == AST_TRAV_LEAVE) {
      string* right = (string*)array_pop(cg_stack);

      stack_append("(%s%s)", psr_operator_str(node->lunary.operator),
                   right->value);
    }
    break;

  case AST_EXPR_RUNARY:
    if (mode == AST_TRAV_LEAVE) {
      string* left = (string*)array_pop(cg_stack);

      stack_append("(%s%s)", left->value,
                   psr_operator_str(node->runary.operator));
    }
    break;
  case AST_EXPR_MEMBER:
    if (mode == AST_TRAV_LEAVE) {
      string* right = (string*)array_pop(cg_stack);
      string* left = (string*)array_pop(cg_stack);
      if (ty_is_struct(node->member.left->ty_id)) {
        // we can be aliasing, so we can trust right->value
        // instead we just use the property name of the type
        ty_t type = ty(node->member.left->ty_id);

        // stack_append("(%s.%s)", left->value, right->value);
        stack_append(
            "(%s.%s)", left->value,
            ((string**)type.structure.members.values)[node->member.idx]->value);
      } else {
        stack_append("(%s[%s])", left->value, right->value);
      }
    }
    break;
  case AST_DTOR_VAR:
    if (mode == AST_TRAV_LEAVE) {
      string* id = (string*)array_pop(cg_stack);
      if (node->var.scope == AST_SCOPE_GLOBAL) {
        // TODO REVIEW extern may need necessary
        // there is no ffi var atm.
        stack_append("/* globvar %s */", id->value);
        CG_OUTPUT(cg_fds->decls, "globvar %s %s;\n",
                  cg_type(node->ty_id)->value, id->value);
      } else {
        // TODO REVIEW the initialization can be done in one step
        if (ty_is_struct(node->ty_id)) {
          stack_append("%s %s = {0}", cg_type(node->ty_id)->value, id->value);
        } else if (ty_is_pointer_like(node->ty_id)) {
          stack_append("%s %s = 0", cg_type(node->ty_id)->value, id->value);
        } else {
          stack_append("%s %s", cg_type(node->ty_id)->value, id->value);
        }
      }
    }
    break;
  /*
  case AST_TYPE:
    break;
  */
  case AST_DECL_STRUCT:
    // if it's a template, cannot be in c
    if (node->structure.tpls != 0) {
      return AST_SEARCH_SKIP;
    }

    if (mode == AST_TRAV_ENTER) {
      node->stack = cg_stack->length;
    } else {
      int buffer_idx = 0;
      buffer2[0] = 0;
      string* id = (string*)cg_stack->values[node->stack];
      // TODO REVIEW skip the first one is the identifier atm
      for (int i = node->stack + 1; i < cg_stack->length; ++i) {
        string* arg = (string*)cg_stack->values[i];

        buffer_idx += snprintf(buffer2 + buffer_idx, 1024, "%s;\n", arg->value);
      }
      cg_stack->length = node->stack;

      CG_OUTPUT(cg_fds->decls, "struct %s__cg;\n", id->value);
      CG_OUTPUT(cg_fds->decls, "typedef struct %s__cg %s;\n", id->value,
                id->value);
      CG_OUTPUT(cg_fds->types, "struct %s__cg {\n%s\n};\n\n", id->value,
                buffer2);
    }
    break;
  case AST_DECL_STRUCT_FIELD:
    if (mode == AST_TRAV_LEAVE) {
      string* id = (string*)array_pop(cg_stack);
      stack_append("%s %s", cg_type(node->ty_id)->value, id->value);
    }
    break;
  case AST_DECL_FUNCTION:
    if (node->func.templated) {
      // templated function are not exported, it's clones are
      return AST_SEARCH_SKIP;
    }

    if (mode == AST_TRAV_ENTER) {
      u64 current = cg_stack->length;
      int buffer_idx = 0;
      buffer[0] = 0;

      if (node->func.params->list.length) {
        u64 i = 0;
        ast_t* tmp;
        ast_t** itr = node->func.params->list.values;

        while ((tmp = itr[i++]) != 0) {
          ast_traverse(tmp, __codegen_cb, 0, 0, 0, 0);
          string* param = (string*)array_pop(cg_stack);

          // stack_append("%s %s", cg_type(tmp->ty_id)->value, param->value);
          if (itr[i] == 0) {
            buffer_idx += snprintf(buffer + buffer_idx, 1024, "%s %s",
                                   cg_type(tmp->ty_id)->value, param->value);
          } else {
            buffer_idx += snprintf(buffer + buffer_idx, 1024, "%s %s, ",
                                   cg_type(tmp->ty_id)->value, param->value);
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

        string* body = (string*)array_pop(cg_stack);

        CG_OUTPUT(cg_fds->decls, "%s %s (%s);\n",
                  cg_type(node->func.ret_type->ty_id)->value,
                  node->func.uid->value, parameters->value);
        // TODO function must not end with: __fnptr
        CG_OUTPUT(cg_fds->types, "typedef %s (*%s__fnptr) (%s);\n",
                  cg_type(node->func.ret_type->ty_id)->value,
                  node->func.uid->value, parameters->value);

        if (node->func.type == AST_FUNC_OPERATOR ||
            node->func.type == AST_FUNC_PROPERTY) {
          CG_OUTPUT(cg_fds->functions, "force_inline ");
        }

        CG_OUTPUT(cg_fds->functions, "%s %s (%s) %s",
                  cg_type(node->func.ret_type->ty_id)->value,
                  node->func.uid->value, parameters->value, body->value);
      } else {
        // TODO skip this processs atm
        return AST_SEARCH_SKIP;

        stack_append("extern %s %s (%s)",
                     cg_type(node->func.ret_type->ty_id)->value,
                     node->func.uid->value, parameters->value);
      }

      return AST_SEARCH_SKIP;
    }

    break;

  case AST_EXPR_CALL:
    if (node->parent->type == AST_IMPLEMENT) {
      return AST_SEARCH_SKIP;
    }

    if (mode == AST_TRAV_ENTER) {
      node->stack = cg_stack->length;
    } else {
      int buffer_idx = 0;
      buffer2[0] = 0;
      // TODO REVIEW skip the first one is the identifier atm
      for (int i = node->stack + 1; i < cg_stack->length; ++i) {
        string* arg = (string*)cg_stack->values[i];

        if (i == cg_stack->length - 1) {
          buffer_idx += snprintf(buffer2 + buffer_idx, 1024, "%s", arg->value);
        } else {
          buffer_idx +=
              snprintf(buffer2 + buffer_idx, 1024, "%s, ", arg->value);
        }
      }
      cg_stack->length = node->stack;

      stack_append("%s(%s)", node->call.decl->func.uid->value, buffer2);
    }

    break;

  case AST_DECL_TEMPLATE:
    return AST_SEARCH_SKIP;
    break;
  case AST_STMT_RETURN:
    if (mode == AST_TRAV_LEAVE) {
      if (node->ret.argument) {
        string* expr = (string*)array_pop(cg_stack);

        stack_append("return %s", expr->value);
      } else {
        stack_append("return"); // ; ?
      }
    }
    break;
  /*
  case AST_ERROR:
    break;
  */
  case AST_STMT_COMMENT:
    stack_append("/* %s */", node->comment.text->value);
    return AST_SEARCH_SKIP; // once
    break;
  case AST_STMT_IF: {
    string* block = cg_node(node->if_stmt.block);
    string* test = cg_node(node->if_stmt.test);
    if (!node->if_stmt.alternate) {
      stack_append("if (%s) %s", test->value, block->value);
    } else {
      string* alternate = cg_node(node->if_stmt.alternate);
      stack_append("if (%s) %s else %s", test->value, block->value,
                   alternate->value);
    }
    return AST_SEARCH_SKIP; // manual traverse
  } break;
  case AST_STMT_LOOP:
    // traverse order: init, pre_cond, update, block, post_cond
    switch (node->loop.type) {
    case AST_STMT_FOR: {
      string* init = cg_node(node->loop.init);
      string* pre_cond = cg_node(node->loop.pre_cond);
      string* update = cg_node(node->loop.update);
      string* block = cg_node(node->loop.block);
      stack_append("for (%s; %s; %s) %s", init->value, pre_cond->value,
                   update->value, block->value);
    } break;
    case AST_STMT_WHILE: {
      string* pre_cond = cg_node(node->loop.pre_cond);
      string* block = cg_node(node->loop.block);
      stack_append("while (%s) %s", pre_cond->value, block->value);
    } break;
    case AST_STMT_DOWHILE: {
      string* post_cond = cg_node(node->loop.post_cond);
      string* block = cg_node(node->loop.block);
      stack_append("do %s while (%s)", block->value, post_cond->value);
    } break;
    default: { fl_assert(0); }
    }
    return AST_SEARCH_SKIP; // manual traverse
    break;
  case AST_EXPR_SIZEOF:
    // TODO handle typeof a variable
    fl_assert(node->sof.type->type == AST_TYPE);
    stack_append("sizeof(%s)", cg_type(node->sof.type->ty_id)->value);
    return AST_SEARCH_SKIP; // manual traverse
    break;
  /*
case AST_STMT_LOG:
  break;
*/
  case AST_CAST:
    if (mode == AST_TRAV_LEAVE) {
      string* expr = (string*)array_pop(cg_stack);
      stack_append("((%s) %s)", cg_type(node->ty_id)->value, expr->value);
    }

    break;
  /*
  case AST_ATTRIBUTE:
    break;
    */
  default: { printf("// node ignored?! %d\n", node->type); }
  }

  return AST_SEARCH_CONTINUE;
}

string* cg_node(ast_t* node) {
  node->stack = cg_stack->length;
  ast_traverse(node, __codegen_cb, node, 0, 0, 0);
  int buffer_size;
  for (int i = node->stack; i < cg_stack->length; ++i) {
    buffer_size += ((string*)cg_stack->values[i])->length;
  }

  string* node_str = st_new(buffer_size, st_enc_utf8);
  for (int i = node->stack; i < cg_stack->length; ++i) {
    st_append(&node_str, cg_stack->values[i]);
  }
  cg_stack->length = node->stack; // restore

  return node_str;
}

void cg_type_table(ast_t* root) {
  CG_OUTPUT(cg_fds->types, "type_t types[] = {\n");
  for (int i = 0; i < ts_type_size_s; ++i) {
    CG_OUTPUT(cg_fds->types, "{// id: %d\n", i);
    ty_t type = ty(i);
    if (type.id != 0) {
      st_dump_header(type.id, buffer2);
      CG_OUTPUT(cg_fds->types, ".id = (string*)\"%s\" \"%s\",\n", buffer2,
                type.id->value);
    } else {
      // WTF!
      CG_OUTPUT(cg_fds->types, ".id = (string*)0,\n");
    }
    CG_OUTPUT(cg_fds->types, ".of = %d,\n", type.of);
    switch (type.of) {
    case TY_NUMBER:
      CG_OUTPUT(cg_fds->types, ".number.bits = %d,\n", type.number.bits);
      CG_OUTPUT(cg_fds->types, ".number.fp = %s,\n",
                type.number.fp ? "true" : "false");
      CG_OUTPUT(cg_fds->types, ".number.sign = %s,\n",
                type.number.sign ? "true" : "false");
      break;
    default: {} // TODO handle the rest of type table
    }
    CG_OUTPUT(cg_fds->types, "},\n");
  }

  // number

  CG_OUTPUT(cg_fds->types, "};\n");
}

char* fl_codegen(ast_t* root) {
  // log_debug_level = 10;
  // ty_dump_table();
  // ast_dump(root);

  cg_stack = calloc(sizeof(array), 1);
  buffer = calloc(sizeof(char), 1024);
  buffer2 = calloc(sizeof(char), 1024);
  // array_newcap(cg_stack, 500);
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
                         "#include \"functions.c\"\n\n");

  cg_type_table(root);
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
