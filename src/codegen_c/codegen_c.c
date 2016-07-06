#include "flang/common.h"
#include "flang/libast.h"
#include "flang/typesystem.h"
#include "flang/codegen_c.h"
#include "flang/debug.h"

//#define CG_DEBUG

char* cg_text = 0;

char* buffer = 0;
char* buffer2 = 0;
array* cg_stack;
u64 max_level = 0;
int cg_indent = 0;

#define stack_append(format, ...) \
do {\
  string* str; \
  scan_string(str, format, ##__VA_ARGS__); \
  array_append(cg_stack, (void*) str); \
} while(false);




#define scan_string(str_var, format, ...) \
  snprintf(buffer, 1024, format, ##__VA_ARGS__); \
  str_var = st_newc(buffer, st_enc_utf8); \


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
  printf("%*s", (int)level, " ");
  printf("// ");
  ast_dump_one(node);
  printf("\n");

  for (int i = 0; i < cg_stack->size; ++i) {
    printf("// stack[%d] %s\n", i, ((string*)cg_stack->data[i])->value);
  }
#endif
}

ast_action_t __codegen_cb(ast_trav_mode_t mode, ast_t* node, ast_t* parent, u64 level,
          void* userdata_in, void* userdata_out) {


  assert(node != 0);

  //if (mode == AST_TRAV_ENTER) { cg_dbg(node, level); }
  //cg_dbg(node, level);

  switch (node->type) {
  case AST_PROGRAM:
  case AST_IMPORT:
  case AST_MODULE:
  case AST_LIST:
  case AST_TYPE:
  case AST_PARAMETER:
    break;
  case AST_BLOCK:
    if (mode == AST_TRAV_ENTER) {
      ++cg_indent;
      node->stack = cg_stack->size;
      printf("%*s{\n", cg_indent, " ");
    } else {

      for (int i = node->stack; i < cg_stack->size; ++i) {
        printf("%*s%s;\n", cg_indent, " ",
          ((string*)cg_stack->data[i])->value);
      }
      cg_stack->size = node->stack;

      printf("%*s}\n", cg_indent, " ");
      --cg_indent;
    }

    // traverse do not follow scope hashes
    // so we print it here ??
    //printf("block [%d]", node->block.scope);
    //printf(" tys [%s]", __ast_block_hash_append(node->block.types));
    //printf(" vars [%s]", __ast_block_hash_append(node->block.variables));
    //printf(" fns [%s]", __ast_block_hash_append(node->block.functions));
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

    stack_append("%f", node->decimal.value);
  } break;

  case AST_LIT_IDENTIFIER:
    if (mode == AST_TRAV_LEAVE) return 0;
    if (parent->type == AST_TYPE) return AST_SEARCH_CONTINUE;
    if (parent->type == AST_DECL_FUNCTION) return AST_SEARCH_CONTINUE;
    if (parent->type == AST_PARAMETER) return AST_SEARCH_CONTINUE;
    if (parent->type == AST_EXPR_CALL) return AST_SEARCH_CONTINUE;

    assert(node->identifier.string != 0);
    array_append(cg_stack, (void*) node->identifier.string);
    break;

  case AST_LIT_STRING:
    if (mode == AST_TRAV_LEAVE) return 0;

    // TODO add quotes
    array_append(cg_stack, (void*) node->string.value);
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
  /*
  case AST_EXPR_MEMBER:
    printf("member T(%zu) idx(%zu) expression(%d)", node->ty_id,
           node->member.idx, node->member.expression);
    break;
  */
  case AST_DTOR_VAR:
    if (mode == AST_TRAV_LEAVE) {
      string* id = (string*) array_pop(cg_stack);

      stack_append("%s %s", ty_to_string(node->ty_id)->value, id->value);
    }
    break;
  /*
  case AST_TYPE:
    printf("type T(%zu)", node->ty_id);
    break;
  case AST_DECL_STRUCT:
    printf("struct T(%zu)", node->ty_id);
    break;
  case AST_DECL_STRUCT_FIELD:
    printf("field T(%zu)", node->ty_id);
    break;
  */
  case AST_DECL_FUNCTION:
    if (node->func.templated) {
      // templated function are not exported, it's clones are
      return AST_SEARCH_CONTINUE;
    }

    if (mode == AST_TRAV_ENTER) {
      u64 current = cg_stack->size;
      if (node->func.params->list.count) {
        u64 i = 0;
        ast_t* tmp;
        ast_t** itr = node->func.params->list.elements;

        while ((tmp = itr[i++]) != 0) {
          stack_append("%s %s", ty_to_string(tmp->ty_id)->value,
            tmp->param.id->identifier.string->value);
        }
      }

      int buffer_idx = 0;
      buffer[0] = 0;
      for (int i = current; i < cg_stack->size; ++i) {
        if (i == cg_stack->size - 1) {
          buffer_idx += snprintf(buffer + buffer_idx, 1024, "%s", ((string*)cg_stack->data[i])->value);
        } else {
          buffer_idx += snprintf(buffer + buffer_idx, 1024, "%s, ", ((string*)cg_stack->data[i])->value);
        }
      }

      if (node->func.ffi) {
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

      printf("%*s%s %s (%s)\n",
        cg_indent, " ",
        ty_to_string(node->func.ret_type->ty_id)->value,
        node->func.id->identifier.string->value,
        parameters->value
      );
    }

    break;

    case AST_EXPR_CALL:

    if (mode == AST_TRAV_ENTER) {
      node->stack = cg_stack->size;
    } else {
      for (int i = node->stack; i < cg_stack->size; ++i) {
        printf("%*s%s;\n", cg_indent, " ",
        ((string*)cg_stack->data[i])->value);
      }

      int buffer_idx = 0;
      buffer2[0] = 0;
      for (int i = node->stack; i < cg_stack->size; ++i) {
        string* arg = (string*) cg_stack->data[i];

        if (i == cg_stack->size - 1) {
          buffer_idx += snprintf(buffer2 + buffer_idx, 1024, "%s", arg->value);
        } else {
          buffer_idx += snprintf(buffer2 + buffer_idx, 1024, "%s, ", arg->value);
        }
      }
      cg_stack->size = node->stack;

      stack_append("%s(%s)", node->call.decl->func.id->identifier.string->value, buffer2);
    }

    break;
  /*
  case AST_DECL_TEMPLATE:
    printf("template");
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
    printf("ERROR [%s: %s]", node->err.type->value, node->err.message->value);
    break;
  */
  case AST_STMT_COMMENT:
    if (mode == AST_TRAV_LEAVE) {
      stack_append("/* %s */", node->comment.text->value);
    }
    break;
  /*
  case AST_STMT_IF:
    printf("if");
    break;
  case AST_STMT_LOOP:
    printf("loop");
    break;
  case AST_EXPR_SIZEOF:
    printf("sizeof T(%zu)", node->ty_id);
    break;
  case AST_STMT_LOG:
    printf("log");
    break;
  case AST_CAST:
    printf("cast T(%zu) O(%u)", node->ty_id, node->cast.operation);
    break;
  case AST_ATTRIBUTE:
    printf("attribute");
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

  printf("\n\n\n//codegen\n");
  ast_traverse(root, __codegen_cb, 0, 0, 0, 0);

  free(cg_stack);
  free(buffer);
  free(buffer2);

  return 0;
}
