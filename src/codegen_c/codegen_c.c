#include "flang/common.h"
#include "flang/libast.h"
#include "flang/codegen_c.h"

char* cg_text = 0;

char** closing;
u64 max_level = 0;

char* readable_operator(int operator) {
/*
TK_DOTDOTDOT
TK_DOTDOT
TK_PLUSPLUS
TK_MINUSMINUS
*/

}

void codegen_cb(ast_t* node, ast_t* parent, u64 level,
          void* userdata_in, void* userdata_out) {
  assert(node != 0);
  if (max_level > level) {
    // print closings
    for (int i = max_level; i > level; --i) {
      if (closing[level] != 0) {
        printf("%s", closing[level]);
        closing[level] = 0;
      }
    }
  }
  max_level = level;

  switch (node->type) {
  case FL_AST_PROGRAM:
    printf("// program \n");
    break;
  case FL_AST_IMPORT:
    printf("// ignore import\n");
    break;
  case FL_AST_MODULE:
    printf("// ignore module\n");
    break;
  case FL_AST_BLOCK:
    printf("{\n");
    closing[level] = "}\n";
    // traverse do not follow scope hashes
    // so we print it here
    //printf("block [%d]", node->block.scope);
    //printf(" tys [%s]", __ast_block_hash_append(node->block.types));
    //printf(" vars [%s]", __ast_block_hash_append(node->block.variables));
    //printf(" fns [%s]", __ast_block_hash_append(node->block.functions));
    break;
  case FL_AST_LIST:
    printf("// list [count=%zu]", node->list.count);
    break;
  case FL_AST_EXPR_ASSIGNAMENT:
    printf("assignament T(%zu)", node->ty_id);
    break;
    /*
  case FL_AST_EXPR_BINOP:
    if (node->binop.operator<127) {
      printf("binop T(%zu) [operator=%c]", node->ty_id, node->binop.operator);
    } else {
      printf("binop T(%zu) [operator=%d]", node->ty_id, node->binop.operator);
    }
    break;
  case FL_AST_LIT_INTEGER:
    printf("integer T(%zu) [u=%ld] [zu=%zu]", node->ty_id,
           node->integer.signed_value, node->integer.unsigned_value);
    break;
  case FL_AST_LIT_FLOAT:
    printf("float T(%zu) [f=%f]", node->ty_id, node->decimal.value);
    break;
  case FL_AST_LIT_IDENTIFIER:
    printf("identifier T(%zu) [resolve=%d string=%s]", node->ty_id,
           node->identifier.resolve, node->identifier.string->value);
    break;
  case FL_AST_LIT_STRING:
    printf("string T(%zu) [string=%s]", node->ty_id, node->string.value->value);
    break;
  case FL_AST_LIT_BOOLEAN:
    printf("boolean T(%zu) [value=%d]", node->ty_id, node->boolean.value);
    break;
  case FL_AST_EXPR_LUNARY:
    if (node->lunary.operator<127) {
      printf("lunary T(%zu) [operator=%c]", node->ty_id, node->lunary.operator);
    } else {
      printf("lunary T(%zu) [operator=%d]", node->ty_id, node->lunary.operator);
    }
    break;
  case FL_AST_EXPR_RUNARY:
    if (node->runary.operator<127) {
      printf("runary T(%zu) [operator=%d]", node->ty_id, node->runary.operator);
    } else {
      printf("runary T(%zu) [operator=%c]", node->ty_id, node->runary.operator);
    }
    break;
  case FL_AST_EXPR_CALL:
    printf("call T(%zu) [arguments=%zu]", node->ty_id, node->call.narguments);
    break;
  case FL_AST_EXPR_MEMBER:
    printf("member T(%zu) idx(%zu) expression(%d)", node->ty_id,
           node->member.idx, node->member.expression);
    break;
  case FL_AST_DTOR_VAR:
    printf("variable T(%zu) scope(%s)", node->ty_id,
           node->var.scope == AST_SCOPE_BLOCK ? "block" : "global");
    break;
  case FL_AST_TYPE:
    printf("type T(%zu)", node->ty_id);
    break;
  case FL_AST_DECL_STRUCT:
    printf("struct T(%zu)", node->ty_id);
    break;
  case FL_AST_DECL_STRUCT_FIELD:
    printf("field T(%zu)", node->ty_id);
    break;
  case FL_AST_DECL_FUNCTION:
    printf("function T(%zu) id(%s) uid(%s) ffi(%d) varargs(%d) tpl(%d) "
           "[params=%zu]",
           node->ty_id, node->func.id->identifier.string->value,
           node->func.uid ? node->func.uid->value : "(nil)", node->func.ffi,
           node->func.varargs, node->func.templated,
           node->func.params->list.count);
    break;
  case FL_AST_DECL_TEMPLATE:
    printf("template");
    break;
  case FL_AST_PARAMETER:
    printf("parameter");
    break;
  case FL_AST_STMT_RETURN:
    printf("return");
    break;
  case FL_AST_ERROR:
    printf("ERROR [%s: %s]", node->err.type->value, node->err.message->value);
    break;
  case FL_AST_STMT_COMMENT:
    printf("comment\n**\n%s\n**\n", node->comment.text->value);
    // printf("comment");
    break;
  case FL_AST_STMT_IF:
    printf("if");
    break;
  case FL_AST_STMT_LOOP:
    printf("loop");
    break;
  case FL_AST_EXPR_SIZEOF:
    printf("sizeof T(%zu)", node->ty_id);
    break;
  case FL_AST_STMT_LOG:
    printf("log");
    break;
  case FL_AST_CAST:
    printf("cast T(%zu) O(%u)", node->ty_id, node->cast.operation);
    break;
  case FL_AST_ATTRIBUTE:
    printf("attribute");
    break;
    */
  default: {}
  }
}


char* fl_codegen(ast_t* root) {
  closing = calloc(sizeof(char*), 100);

  printf("// ****************\n");
  ast_traverse(root, codegen_cb, 0, 0, 0, 0);

  free(closing);
}
