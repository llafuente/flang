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

/*
LLVMValueRef F = LLVMAddFunction(M, name, Fty);
LLVMBuilderRef builder = LLVMCreateBuilder();
LLVMPositionBuilderAtEnd(builder, LLVMAppendBasicBlock(F, "entry"));
*/
/*
off = LLVMBuildGEP(builder, param, &stack[depth - 1], 1, "");
stack[depth - 1] = LLVMBuildLoad(builder, off, "");
*/

LLVMModuleRef fl_codegen(ast_t* root, char* module_name) {
  char* err;
  LLVMContextRef context = LLVMGetGlobalContext();
  LLVMModuleRef module =
      // LLVMModuleCreateWithNameInContext(!module_name ? "main" : module_name,
      // context);
      LLVMModuleCreateWithName(!module_name ? "module" : module_name);
  // LLVMBuilderRef builder = LLVMCreateBuilderInContext(context);
  LLVMBuilderRef builder = LLVMCreateBuilder();

  /*
    LLVMExecutionEngineRef jit = cg_jit();

    // Setup optimizations.
    LLVMPassManagerRef pass_manager =
        LLVMCreateFunctionPassManagerForModule(module);
    LLVMAddTargetData(LLVMGetExecutionEngineTargetData(engine), pass_manager);
    LLVMAddPromoteMemoryToRegisterPass(pass_manager);
    LLVMAddInstructionCombiningPass(pass_manager);
    LLVMAddReassociatePass(pass_manager);
    LLVMAddGVNPass(pass_manager);
    LLVMAddCFGSimplificationPass(pass_manager);
    LLVMInitializeFunctionPassManager(pass_manager);
  */

  // create main
  LLVMValueRef parent = LLVMAddFunction(
      module, "main", LLVMFunctionType(LLVMInt32Type(), 0, 0, 0));
  LLVMSetFunctionCallConv(parent, LLVMCCallConv);
  LLVMBasicBlockRef main_block = LLVMAppendBasicBlock(parent, "main-entry");

  LLVMPositionBuilderAtEnd(builder, main_block);

  // node must have parent, because we need to search backwards
  ast_parent(root);
  LLVMBasicBlockRef* current_block;
  current_block = &main_block;
  cg_ast(root, FL_CODEGEN_PASSTHROUGH);

  LLVMBuildRet(builder, LLVMConstInt(LLVMInt32Type(), 0, false));
  // optimize main
  // LLVMRunFunctionPassManager(pass_manager, main);

  // LLVMAbortProcessAction?

  if (log_debug_level >= 4) {
    char* irstr = LLVMPrintModuleToString(module);
    puts("LLVMPrintModuleToString\n");
    puts(irstr);
    LLVMDisposeMessage(irstr);
  }

  LLVMVerifyModule(module, LLVMPrintMessageAction, &err);
  if (strlen(err)) {
    log_debug("LLVMVerifyModule");
    log_error("%s", err);
  }
  LLVMDisposeMessage(err);

  // LLVMDumpModule(module);
  // LLVMDisposePassManager(pass_manager);
  LLVMDisposeBuilder(builder);

  return module;
}

LLVMValueRef cg_ast(FL_CODEGEN_HEADER) {

  switch (node->type) {
  case FL_AST_MODULE:
  case FL_AST_PROGRAM:
    if (node->program.core) {
      log_verbose("** program.core **");
      int olog_debug_level = log_debug_level;
      log_debug_level = 0;
      cg_ast(node->program.core, FL_CODEGEN_PASSTHROUGH);
      log_debug_level = olog_debug_level;
    } else {
      log_warning("** program.core not found");
    }
    log_verbose("** program.body **");
    return cg_ast(node->program.body, FL_CODEGEN_PASSTHROUGH);
  case FL_AST_BLOCK: {
    size_t i = 0;
    ast_t* tmp;

    while ((tmp = node->block.body[i++])) {
      log_debug("block %zu", i);
      cg_ast(tmp, FL_CODEGEN_PASSTHROUGH);
    }

    return 0;
  }
  case FL_AST_EXPR_ASSIGNAMENT:
    return cg_assignament(FL_CODEGEN_HEADER_SEND);
  case FL_AST_CAST:
    return cg_cast(FL_CODEGEN_HEADER_SEND);
  case FL_AST_EXPR_BINOP:
    return cg_binop(FL_CODEGEN_HEADER_SEND);
  case FL_AST_EXPR_CALL:
    return cg_expr_call(FL_CODEGEN_HEADER_SEND);
  case FL_AST_LIT_NUMERIC:
    return cg_lit_number(FL_CODEGEN_HEADER_SEND);
  case FL_AST_LIT_BOOLEAN:
    return cg_lit_boolean(FL_CODEGEN_HEADER_SEND);
  case FL_AST_LIT_STRING:
    return cg_lit_string(FL_CODEGEN_HEADER_SEND);
  case FL_AST_LIT_IDENTIFIER:
    return cg_right_identifier(node, FL_CODEGEN_PASSTHROUGH);
  case FL_AST_EXPR_MEMBER:
    return cg_right_member(node, FL_CODEGEN_PASSTHROUGH);
  case FL_AST_DTOR_VAR:
    return cg_dtor_var(FL_CODEGEN_HEADER_SEND);
  case FL_AST_DECL_FUNCTION:
    return cg_function(FL_CODEGEN_HEADER_SEND);
  case FL_AST_STMT_RETURN:
    return cg_return(FL_CODEGEN_HEADER_SEND);
  case FL_AST_EXPR_LUNARY:
    return cg_lunary(FL_CODEGEN_HEADER_SEND);
  case FL_AST_STMT_IF:
    return cg_if(FL_CODEGEN_HEADER_SEND);
  case FL_AST_STMT_LOOP:
    return cg_loop(FL_CODEGEN_HEADER_SEND);
  case FL_AST_DECL_STRUCT:
    // ignore it, will be created on first use
    break;
  default: {}
    // log_error("(codegen) ast->type not handled %d", node->type);
  }
  return 0;
}

LLVMValueRef cg_do_block(LLVMBasicBlockRef block, LLVMBasicBlockRef fall_block,
                         FL_CODEGEN_HEADER) {
  LLVMBasicBlockRef old_cb = *current_block;

  *current_block = block;
  LLVMPositionBuilderAtEnd(builder, *current_block); // new block
  // codegen
  LLVMValueRef rnode = cg_ast(node, FL_CODEGEN_PASSTHROUGH);
  if (fall_block) {
    LLVMBuildBr(builder, fall_block);
  }
  // restore
  *current_block = old_cb;
  LLVMPositionBuilderAtEnd(builder, *current_block);

  return rnode;
}

LLVMValueRef cg_cast(FL_CODEGEN_HEADER) {
  // check if rigt side is a constant
  ast_t* el = node->cast.element;

  ast_dump(node);
  log_debug("%d - %d", el->type, FL_AST_LIT_NUMERIC);

  if (el->type == FL_AST_LIT_NUMERIC) {
    log_debug("cast: override numeric type T(%zu)", node->ty_id);
    node->cast.element->ty_id = node->ty_id;

    return cg_ast(el, FL_CODEGEN_PASSTHROUGH);
  }

  LLVMValueRef element = cg_ast(el, FL_CODEGEN_PASSTHROUGH);

  return cg_cast_op(builder, ast_get_typeid(el), node->ty_id, element, context);
}

LLVMValueRef cg_lit_number(FL_CODEGEN_HEADER) {
  log_debug("number T(%zu)", node->ty_id);

  // get parent type, to know what type should i be.
  size_t ty = node->ty_id;
  // size_t ty = node->numeric.ty_id;
  ty_t t = ts_type_table[ty];

  if (t.number.fp) {
    return LLVMConstReal(cg_get_typeid(ty, context), node->numeric.value);
  }

  return LLVMConstInt(cg_get_typeid(ty, context), node->numeric.value,
                      t.number.sign);
}

LLVMValueRef cg_lit_boolean(FL_CODEGEN_HEADER) {
  ty_t t = ts_type_table[2];

  return LLVMConstInt(cg_get_typeid(2, context), node->boolean.value, false);
}

LLVMValueRef cg_lit_string(FL_CODEGEN_HEADER) {
  log_debug("cg_lit_string");

  return LLVMBuildGlobalStringPtr(builder, node->string.value->value,
                                  "string_val" // TODO must be unique!
                                  );
}

LLVMValueRef cg_assignament(FL_CODEGEN_HEADER) {
  log_debug("right");
  LLVMValueRef right = cg_ast(node->assignament.right, FL_CODEGEN_PASSTHROUGH);

  log_debug("left");
  ast_t* l = node->assignament.left;
  LLVMValueRef left = cg_lhs(l, FL_CODEGEN_PASSTHROUGH);

  assert(left != 0);
  assert(right != 0);

  LLVMValueRef assign = LLVMBuildStore(builder, right, left);

  left = LLVMBuildLoad(builder, left, "load_l_ass");
  // if left is an identifier, set
  // if (l->type == FL_AST_LIT_IDENTIFIER) {
  //  ast_t* id = ast_search_decl_var(node, l->identifier.string);
  //  id->last_codegen = left;
  //}
  return left;
}

// https://msdn.microsoft.com/en-us/library/09ka8bxx.aspx
LLVMValueRef cg_binop(FL_CODEGEN_HEADER) {
  log_debug("cg_binop");

  ast_t* l = node->binop.left;
  ast_t* r = node->binop.right;

  // retrieve left/right side
  ast_dump(l);
  LLVMValueRef lhs = cg_ast(l, FL_CODEGEN_PASSTHROUGH);
  if (!lhs) {
    log_error("invalid lhs");
  }

  ast_dump(r);
  LLVMValueRef rhs = cg_ast(r, FL_CODEGEN_PASSTHROUGH);
  if (!rhs) {
    log_error("invalid rhs");
  }

  log_verbose("using binop %d", node->binop.operator);

  // common operations that works with any type
  switch (node->binop.operator) {
  case FL_TK_AND:
    return LLVMBuildAnd(builder, lhs, rhs, "and");
  case FL_TK_OR:
    return LLVMBuildOr(builder, lhs, rhs, "or");
  case FL_TK_CARET:
    return LLVMBuildXor(builder, lhs, rhs, "xor");
  case FL_TK_LT2:
    // returns the first operand shifted to the left a specified number of bits.
    return LLVMBuildShl(builder, lhs, rhs, "shl");
  case FL_TK_GT2:
    // returns the first operand shifted to the right a specified number of bits
    // with sign extension
    return LLVMBuildAShr(builder, lhs, rhs, "ashr");

  // TODO
  // logical shift right - lshr
  // return LLVMBuildLShr(builder, lhs, rhs, "lshr");
  // signed
  default: {}
  }

  bool use_fp = ts_is_fp(node->ty_id);
  // Create different IR code depending on the operator.
  switch (node->binop.operator) {
  case FL_TK_EQUAL2: { // ==
    return use_fp ? LLVMBuildFCmp(builder, LLVMRealOEQ, lhs, rhs, "andand")
                  : LLVMBuildICmp(builder, LLVMIntEQ, lhs, rhs, "andand");
  }
  case FL_TK_EEQUAL: { // !=
    return use_fp ? LLVMBuildFCmp(builder, LLVMRealONE, lhs, rhs, "andand")
                  : LLVMBuildICmp(builder, LLVMIntNE, lhs, rhs, "andand");
  }
  case FL_TK_GT: { // >
    // TODO LLVMIntUGT
    // TODO LLVMRealUGT

    return use_fp ? LLVMBuildFCmp(builder, LLVMRealOGT, lhs, rhs, "andand")
                  : LLVMBuildICmp(builder, LLVMIntSGT, lhs, rhs, "andand");
  }
  case FL_TK_GTE: { // >=
    // TODO LLVMIntUGE
    // TODO LLVMRealUGE

    return use_fp ? LLVMBuildFCmp(builder, LLVMRealOGE, lhs, rhs, "andand")
                  : LLVMBuildICmp(builder, LLVMIntSGE, lhs, rhs, "andand");
  }
  case FL_TK_LT: { // <
    // TODO LLVMIntULT
    // TODO LLVMRealULT

    return use_fp ? LLVMBuildFCmp(builder, LLVMRealOLT, lhs, rhs, "andand")
                  : LLVMBuildICmp(builder, LLVMIntSLT, lhs, rhs, "andand");
  }
  case FL_TK_LTE: { // <=
    // TODO LLVMIntULE
    // TODO LLVMRealULE

    return use_fp ? LLVMBuildFCmp(builder, LLVMRealOLE, lhs, rhs, "andand")
                  : LLVMBuildICmp(builder, LLVMIntSLE, lhs, rhs, "andand");
  }
  /*
    https://github.com/VoltLang/Volta/blob/8ba031a16f742262c8eeb1ba2eed8670100c576c/src/volt/llvm/expression.d#L275
    http://llvm.org/docs/doxygen/html/Core_8h.html
    ugt: unsigned greater than
    uge: unsigned greater or equal
    ult: unsigned less than
    ule: unsigned less or equal
    sgt: signed greater than
    sge: signed greater or equal
    slt: signed less than
    sle: signed less or equal
  */

  case FL_TK_PLUS: {
    return use_fp ? LLVMBuildFAdd(builder, lhs, rhs, "add")
                  : LLVMBuildAdd(builder, lhs, rhs, "addi");
  }
  case FL_TK_MINUS: {
    return use_fp ? LLVMBuildFSub(builder, lhs, rhs, "sub")
                  : LLVMBuildSub(builder, lhs, rhs, "subi");
  }
  case FL_TK_ASTERISK: {
    return use_fp ? LLVMBuildFMul(builder, lhs, rhs, "mul")
                  : LLVMBuildMul(builder, lhs, rhs, "muli");
  }
  case FL_TK_SLASH: {
    // signed vs unsigned
    return use_fp ? LLVMBuildFDiv(builder, lhs, rhs, "div")
                  : LLVMBuildSDiv(builder, lhs, rhs, "divi");
  }
  case FL_TK_MOD: {
    return use_fp ? LLVMBuildFRem(builder, lhs, rhs, "mod")
                  : LLVMBuildSRem(builder, lhs, rhs, "modi");
  }

  default: {}
  }

  log_error("(codegen) binop not supported: %d", node->binop.operator);

  return 0;
}

// TODO manage type
LLVMValueRef cg_dtor_var(FL_CODEGEN_HEADER) {
  log_debug("cg_dtor_var");

  // TODO use ty_id
  LLVMValueRef ref =
      LLVMBuildAlloca(builder, cg_get_type(node->var.type, context),
                      node->var.id->identifier.string->value);
  // TODO fix me!
  // LLVMSetAlignment(ref, 8);
  node->var.alloca = (void*)ref;

  return ref;
}
// TODO parent rewrite!
LLVMValueRef cg_function(FL_CODEGEN_HEADER) {
  log_debug("cg_function");

  if (!node->func.ret_type) {
    log_error("function has no return type");
  }

  LLVMTypeRef param_types[node->func.nparams];

  // TODO use type info should be faster
  size_t i = 0;
  ast_t* params = node->func.params;
  ast_t* tmp;

  if (node->func.params) {
    while ((tmp = params->list.elements[i]) != 0) {
      if (!tmp->param.id->ty_id) {
        log_error("Parameter %zu don't have type", i);
      }

      log_debug("parameter %zu of type %zu", i, tmp->param.id->ty_id);
      param_types[i++] = cg_get_typeid(tmp->param.id->ty_id, context);
    }
  }

  LLVMTypeRef ret_type =
      LLVMFunctionType(cg_get_type(node->func.ret_type, context), param_types,
                       node->func.nparams, node->func.varargs);

  LLVMValueRef func = LLVMAddFunction(module, node->func.uid->value, ret_type);
  LLVMSetFunctionCallConv(func, LLVMCCallConv);
  LLVMSetLinkage(func, LLVMExternalLinkage);

  i = 0;
  if (node->func.params) {
    while ((tmp = params->list.elements[i]) != 0) {
      log_verbose("set name [%zu] '%s'", i,
                  tmp->param.id->identifier.string->value);
      LLVMValueRef param = LLVMGetParam(func, i);
      LLVMSetValueName(param, tmp->param.id->identifier.string->value);
      /*
      if (node->func.body) {
        LLVMValueRef ref =
            LLVMBuildAlloca(builder, LLVMTypeOf(param),
    tmp->param.id->identifier.string->value);

      LLVMBuildStore(builder, ref, param);
      tmp->param.alloca = (void*)ref;
    } else {
      tmp->param.alloca = (void*)param;

    }
    */
      tmp->param.alloca = (void*)param;

      ++i;
    }
  }

  if (node->func.body) {
    LLVMBasicBlockRef block = LLVMAppendBasicBlock(func, "function-block");
    cg_do_block(block, 0, node->func.body, FL_CODEGEN_PASSTHROUGH);
  }

  return func;
}

LLVMValueRef cg_return(FL_CODEGEN_HEADER) {
  log_debug("cg_return");

  LLVMValueRef argument = cg_ast(node->ret.argument, FL_CODEGEN_PASSTHROUGH);
  LLVMBuildRet(builder, argument);

  return 0;
}

LLVMValueRef cg_expr_call(FL_CODEGEN_HEADER) {
  log_debug("cg_expr_call");

  LLVMValueRef fn =
      LLVMGetNamedFunction(module, node->call.decl->func.uid->value);
  if (!fn) {
    log_error("function [%s] not found in current context",
              node->call.callee->identifier.string->value);
  }

  LLVMValueRef arguments[node->call.narguments];
  LLVMValueRef value;

  if (node->call.narguments) {
    size_t i = 0;
    ast_t* tmp;
    ast_t* list = node->call.arguments;

    while ((tmp = list->list.elements[i]) != 0) {
      log_verbose("argument! %zu", i);
      value = cg_ast(tmp, FL_CODEGEN_PASSTHROUGH);
      if (!value) {
        log_error("argument fail! %zu", i);
      }
      arguments[i++] = value;
    }

    return LLVMBuildCall(builder, fn, arguments, node->call.narguments, "");
  }

  LLVMValueRef* empty_args;
  *empty_args = 0;
  return LLVMBuildCall(builder, fn, empty_args, 0, "");

  /*
  LLVMValueRef argument =
      cg_ast(node->ret.argument, FL_CODEGEN_PASSTHROUGH);
  LLVMBuildRet(builder, argument);

  //LLVMSetInstructionCallConv(result.value, LLVMCallConv.X86Stdcall);

  if (path.landingBlock is null) {
    return LLVMBuildCall(builder, fn, args);
  } else {
    auto b = LLVMAppendBasicBlockInContext(
      context, currentFunc, "");
    auto ret = LLVMBuildInvoke(builder, fn, args, b, path.landingBlock);
    LLVMMoveBasicBlockAfter(b, currentBlock);
    LLVMPositionBuilderAtEnd(builder, b);
    currentBlock = b;
    return ret;
  }
  */
}

LLVMValueRef cg_lunary(FL_CODEGEN_HEADER) {
  log_debug("cg_lunary");

  LLVMValueRef element = cg_ast(node->lunary.element, FL_CODEGEN_PASSTHROUGH);

  switch (node->lunary.operator) {
  case FL_TK_MINUS:
    return LLVMBuildNeg(builder, element, "negate");
  case FL_TK_EXCLAMATION:
    return LLVMBuildNot(builder, element, "not");
  // TODO buggy, need tests!
  case FL_TK_PLUS2: {
    LLVMTypeRef type = LLVMTypeOf(element);
    LLVMValueRef one = LLVMConstInt(type, 1, false);
    LLVMValueRef one_added = LLVMBuildAdd(builder, element, one, "");
    // if can be stored do it
    if (node->lunary.element->type == FL_AST_LIT_IDENTIFIER) {
      node->lunary.element->dirty = true;
      return LLVMBuildStore(builder, one_added,
                            (LLVMValueRef)node->lunary.element->var.alloca);
    }
    return one_added;
  }
  case FL_TK_MINUS2: {
    LLVMTypeRef type = LLVMTypeOf(element);
    LLVMValueRef one = LLVMConstInt(type, 1, false);
    LLVMValueRef one_substracted = LLVMBuildSub(builder, element, one, "");
    return LLVMBuildStore(builder, element, one_substracted);
  }
  case FL_TK_AND: {
    return element; // this should no be loaded!
  }
  default: {}
  }
  log_error("lunary not handled %d", node->lunary.operator);
  return 0;
  /*
  case FL_TK_PLUS:
  case FL_TK_TILDE:
  case FL_TK_DELETE:
  */
}

LLVMValueRef cg_if(FL_CODEGEN_HEADER) {
  log_debug("(codegen) cg_if");
  bool has_else = node->if_stmt.alternate > 0;

  // end at the end
  LLVMBasicBlockRef end_block = LLVMAppendBasicBlock(parent, "if-end");
  LLVMMoveBasicBlockAfter(end_block, *current_block);

  // else in the middle
  LLVMBasicBlockRef if_else_block;
  if (has_else) {
    if_else_block = LLVMAppendBasicBlock(parent, "if-false");
    LLVMMoveBasicBlockAfter(if_else_block, *current_block);
  }

  // then at the top
  LLVMBasicBlockRef if_then_block = LLVMAppendBasicBlock(parent, "if-true");
  LLVMMoveBasicBlockAfter(if_then_block, *current_block);

  // test expression
  LLVMValueRef test = cg_ast(node->if_stmt.test, FL_CODEGEN_PASSTHROUGH);
  LLVMBuildCondBr(builder, test, if_then_block,
                  has_else ? if_else_block : end_block);

  // then block
  cg_do_block(if_then_block, end_block, node->if_stmt.block,
              FL_CODEGEN_PASSTHROUGH);

  if (has_else) {
    LLVMPositionBuilderAtEnd(builder, if_else_block);
    cg_do_block(if_else_block, end_block, node->if_stmt.alternate,
                FL_CODEGEN_PASSTHROUGH);
  }

  LLVMPositionBuilderAtEnd(builder, end_block);
  *current_block = end_block;

  return 0;
}
// manage for, while, do-while
LLVMValueRef cg_loop(FL_CODEGEN_HEADER) {
  log_debug("cg_loop");

  // blocks are backwards!
  LLVMBasicBlockRef pre_cond = 0;
  LLVMBasicBlockRef post_cond = 0;
  LLVMBasicBlockRef update = 0;
  LLVMBasicBlockRef start = LLVMAppendBasicBlock(parent, "loop-start");
  LLVMBasicBlockRef block = LLVMAppendBasicBlock(parent, "loop-block");
  LLVMBasicBlockRef end_block = LLVMAppendBasicBlock(parent, "loop-end");

  LLVMBuildBr(builder, start); // current-block -> start
  // initialization at start
  if (node->loop.init) {
    cg_do_block(start, 0, node->loop.init, FL_CODEGEN_PASSTHROUGH);
  }

  if (node->loop.pre_cond) {
    pre_cond = LLVMAppendBasicBlock(parent, "loop-pre_cond");

    LLVMPositionBuilderAtEnd(builder, start);
    LLVMBuildBr(builder, pre_cond);
    LLVMPositionBuilderAtEnd(builder, *current_block);

    // test expression

    LLVMPositionBuilderAtEnd(builder, pre_cond);
    LLVMValueRef pre_cond_test =
        cg_ast(node->loop.pre_cond, FL_CODEGEN_PASSTHROUGH);
    LLVMBuildCondBr(builder, pre_cond_test, block, end_block);
    LLVMPositionBuilderAtEnd(builder, *current_block);
  } else {
    LLVMPositionBuilderAtEnd(builder, start);
    LLVMBuildBr(builder, block);
    LLVMPositionBuilderAtEnd(builder, *current_block);
  }

  if (node->loop.update) {
    update = LLVMAppendBasicBlock(parent, "loop-update");
    cg_do_block(update, pre_cond, node->loop.update, FL_CODEGEN_PASSTHROUGH);
  }

  if (node->loop.post_cond) {
    post_cond = LLVMAppendBasicBlock(parent, "loop-post_cond");

    // test expression

    LLVMPositionBuilderAtEnd(builder, post_cond);
    LLVMValueRef post_cond_test =
        cg_ast(node->loop.post_cond, FL_CODEGEN_PASSTHROUGH);
    LLVMBuildCondBr(builder, post_cond_test, block, end_block);
    LLVMPositionBuilderAtEnd(builder, *current_block);
  }

  cg_do_block(block, update ? update
                            : (pre_cond ? pre_cond
                                        : (post_cond ? post_cond : end_block)),
              node->loop.block, FL_CODEGEN_PASSTHROUGH);

  LLVMMoveBasicBlockAfter(end_block, *current_block);
  if (update) {
    LLVMMoveBasicBlockAfter(update, *current_block);
  }
  LLVMMoveBasicBlockAfter(block, *current_block);
  if (pre_cond) {
    LLVMMoveBasicBlockAfter(pre_cond, *current_block);
  }
  LLVMMoveBasicBlockAfter(start, *current_block);

  LLVMPositionBuilderAtEnd(builder, end_block);
  *current_block = end_block;

  return 0;
}

LLVMValueRef cg_left_identifier(FL_CODEGEN_HEADER) {
  log_debug("identifier");
  ast_dump(node);
  ast_t* decl = ast_search_decl_var(node, node->identifier.string);
  ast_dump(decl);

  if (!decl) {
    log_error("identifier not found: '%s'", node->identifier.string->value);
  }

  if (decl->type == FL_AST_DTOR_VAR) {
    return (LLVMValueRef)decl->var.alloca;
  }
  if (decl->type == FL_AST_PARAMETER) {
    return (LLVMValueRef)decl->param.alloca;
  }

  log_error("unhandled identifier decl %d", decl->type);

  return 0;
}

LLVMValueRef cg_right_identifier(FL_CODEGEN_HEADER) {
  ast_t* decl = ast_search_decl_var(node, node->identifier.string);

  if (!decl) {
    log_error("identifier not found: '%s'", node->identifier.string->value);
  }

  if (decl->type == FL_AST_DTOR_VAR) {
    if (ts_is_pointer(node->ty_id)) {
      return (LLVMValueRef)decl->var.alloca;
    }
    return LLVMBuildLoad(builder, (LLVMValueRef)decl->var.alloca, "load_dtor");
  }
  if (decl->type == FL_AST_PARAMETER) {
    return (LLVMValueRef)decl->param.alloca;
  }

  log_error("unhandled identifier decl %d", decl->type);

  return 0;
}

LLVMValueRef cg_left_member(FL_CODEGEN_HEADER) {
  ty_t* type = &ts_type_table[node->member.left->ty_id];

  log_verbose("***********************");
  ast_dump(node);
  log_verbose("left is %zu", node->member.left->ty_id);
  ty_dump(node->member.left->ty_id);

  switch (type->of) {
  case FL_STRUCT: {
    LLVMValueRef left = cg_lhs(node->member.left, FL_CODEGEN_PASSTHROUGH);
    ty_t* myt = &ts_type_table[node->member.left->ty_id];

    return LLVMBuildStructGEP(builder, left, node->member.idx, "");
  }
  case FL_POINTER: {
    LLVMValueRef left = cg_ast(node->member.left, FL_CODEGEN_PASSTHROUGH);
    LLVMValueRef index[1];
    left = LLVMBuildLoad(builder, left, "load_ptr");

    if (node->member.expression) {
      index[0] = cg_ast(node->member.property, FL_CODEGEN_PASSTHROUGH);
    }

    return LLVMBuildGEP(builder, left, index, 1, "");
  }
  default: { log_error("wtf?!"); }
  }

  return 0;
}

// TODO is this right?! LLVMBuildLoad depens on type?!
LLVMValueRef cg_right_member(FL_CODEGEN_HEADER) {
  LLVMValueRef r = cg_left_member(node, FL_CODEGEN_PASSTHROUGH);

  ty_t* type = &ts_type_table[node->ty_id];
  switch (type->of) {
  case FL_VECTOR:
  case FL_POINTER:
    return r;
    break;
  default:
    return LLVMBuildLoad(builder, r, "r_member");
  }
}

LLVMValueRef cg_lhs(FL_CODEGEN_HEADER) {
  switch (node->type) {
  case FL_AST_EXPR_MEMBER: {
    return cg_left_member(node, FL_CODEGEN_PASSTHROUGH);
  }
  case FL_AST_LIT_IDENTIFIER: {
    return cg_left_identifier(node, FL_CODEGEN_PASSTHROUGH);
  }
  default: { log_error("invalid lhs"); }
  }

  return 0;
}
