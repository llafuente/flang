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

  log_debug("this is what codegen see");
  ast_dump(root);

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
  LLVMValueRef main_function = LLVMAddFunction(
      module, "main", LLVMFunctionType(LLVMInt32Type(), 0, 0, 0));
  LLVMSetFunctionCallConv(main_function, LLVMCCallConv);
  LLVMBasicBlockRef main_block =
      LLVMAppendBasicBlock(main_function, "main-entry");

  LLVMPositionBuilderAtEnd(builder, main_block);

  // node must have parent, because we need to search backwards
  ast_parent(root);
  LLVMValueRef* parent = &main_function;
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

LLVMValueRef cg_ast_loaded(char* dbg, FL_CODEGEN_HEADER) {
  LLVMValueRef element = cg_ast(node, FL_CODEGEN_PASSTHROUGH);
  assert(element != 0);

  if (ast_require_load(node)) {
    return LLVMBuildLoad(builder, element, dbg);
  }

  return element;
}

LLVMValueRef cg_ast(FL_CODEGEN_HEADER) {

  switch (node->type) {
  case FL_AST_MODULE:
  case FL_AST_PROGRAM:
    log_verbose("** program.body **");
    return cg_ast(node->program.body, FL_CODEGEN_PASSTHROUGH);
  case FL_AST_BLOCK: {
    cg_ast(node->block.body, FL_CODEGEN_PASSTHROUGH);
    return 0;
  }
  case FL_AST_LIST: {
    assert(node->parent->type == FL_AST_BLOCK ||
           node->parent->type == FL_AST_LIST);

    size_t i;
    for (i = 0; i < node->list.count; ++i) {
      log_debug("block %zu", i);
      cg_ast(node->list.elements[i], FL_CODEGEN_PASSTHROUGH);
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
  case FL_AST_LIT_INTEGER:
    return cg_lit_integer(FL_CODEGEN_HEADER_SEND);
  case FL_AST_LIT_FLOAT:
    return cg_lit_float(FL_CODEGEN_HEADER_SEND);
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
  case FL_AST_EXPR_RUNARY:
    return cg_runary(FL_CODEGEN_HEADER_SEND);
  case FL_AST_STMT_IF:
    return cg_if(FL_CODEGEN_HEADER_SEND);
  case FL_AST_STMT_LOOP:
    return cg_loop(FL_CODEGEN_HEADER_SEND);
  case FL_AST_EXPR_SIZEOF:
    return cg_sizeof(FL_CODEGEN_HEADER_SEND);
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
  // save block
  LLVMBasicBlockRef _current_block = *current_block;
  *current_block = block;
  // end save

  LLVMPositionBuilderAtEnd(builder, block); // new block
  // codegen
  LLVMValueRef rnode = cg_ast(node, FL_CODEGEN_PASSTHROUGH);
  if (fall_block) {
    LLVMBuildBr(builder, fall_block);
  }

  // restore block
  LLVMPositionBuilderAtEnd(builder, _current_block);
  *current_block = _current_block;
  // end restore

  return rnode;
}

LLVMValueRef cg_cast(FL_CODEGEN_HEADER) {
  LLVMValueRef element = cg_ast_loaded("loaded_cast_el", node->cast.element,
                                       FL_CODEGEN_PASSTHROUGH);

  switch (node->cast.operation) {
  case FL_CAST_ERR: {
    ast_dump(node);
    log_error("casting operation not set by typesystem!");
    return 0;
  }
  case FL_CAST_FPTOSI:
    return LLVMBuildFPToSI(builder, element,
                           cg_get_typeid(node->ty_id, context), "cast");
  case FL_CAST_FPTOUI:
    return LLVMBuildFPToUI(builder, element,
                           cg_get_typeid(node->ty_id, context), "cast");
  case FL_CAST_SITOFP:
    return LLVMBuildSIToFP(builder, element,
                           cg_get_typeid(node->ty_id, context), "cast");
  case FL_CAST_UITOFP:
    return LLVMBuildUIToFP(builder, element,
                           cg_get_typeid(node->ty_id, context), "cast");
  case FL_CAST_FPEXT:
    return LLVMBuildFPExt(builder, element, cg_get_typeid(node->ty_id, context),
                          "cast");
  case FL_CAST_SEXT:
    return LLVMBuildSExt(builder, element, cg_get_typeid(node->ty_id, context),
                         "cast");
  case FL_CAST_ZEXT:
    return LLVMBuildZExt(builder, element, cg_get_typeid(node->ty_id, context),
                         "cast");
  case FL_CAST_FPTRUNC:
    return LLVMBuildFPTrunc(builder, element,
                            cg_get_typeid(node->ty_id, context), "cast");
  case FL_CAST_TRUNC:
    return LLVMBuildTrunc(builder, element, cg_get_typeid(node->ty_id, context),
                          "cast");
  case FL_CAST_BITCAST:
    return LLVMBuildBitCast(builder, element,
                            cg_get_typeid(node->ty_id, context), "cast");
  // remove warning!
  case FL_CAST_AUTO: {
    return element;
  }
  }
}

LLVMValueRef cg_lit_float(FL_CODEGEN_HEADER) {
  log_debug("float T(%zu)", node->ty_id);

  LLVMTypeRef tref = cg_get_typeid(node->ty_id, context);
  return LLVMConstReal(tref, node->decimal.value);
}

LLVMValueRef cg_lit_integer(FL_CODEGEN_HEADER) {
  log_debug("integer T(%zu)", node->ty_id);

  // get parent type, to know what type should i be.
  size_t ty = node->ty_id;
  // size_t ty = node->numeric.ty_id;
  ty_t t = ts_type_table[ty];

  LLVMTypeRef tref = cg_get_typeid(ty, context);

  // TODO happens on cast, this should be promoted to AST_LIT_DECIMAL?
  if (t.number.fp) {
    return LLVMConstReal(tref, node->integer.signed_value
                                   ? node->integer.signed_value
                                   : node->integer.unsigned_value);
  }

  if (t.number.sign) {
    return LLVMConstInt(tref, node->integer.signed_value, t.number.sign);
  }

  return LLVMConstInt(tref, node->integer.unsigned_value, t.number.sign);
}

LLVMValueRef cg_lit_boolean(FL_CODEGEN_HEADER) {
  ty_t t = ts_type_table[2];

  return LLVMConstInt(cg_get_typeid(2, context), node->boolean.value, false);
}

LLVMValueRef cg_lit_string(FL_CODEGEN_HEADER) {
  log_debug("cg_lit_string");

  // TODO return LLVMBuildGlobalStringPtr(builder, st_dump(node->string.value),
  // "str");
  return LLVMBuildGlobalStringPtr(builder, node->string.value->value, "str");
}

LLVMValueRef cg_assignament(FL_CODEGEN_HEADER) {
  if (node->assignament.operator != '=') {
    ast_raise_error(node, "invalid assignament for codegen. Must be expanded.");
  }

  log_debug("right");
  ast_t* r = node->assignament.right;
  LLVMValueRef right =
      cg_ast_loaded("load_assignament", r, FL_CODEGEN_PASSTHROUGH);

  log_debug("left");
  ast_t* l = node->assignament.left;
  LLVMValueRef left = cg_lhs(l, FL_CODEGEN_PASSTHROUGH);

  assert(left != 0);
  assert(right != 0);

  LLVMValueRef assign = LLVMBuildStore(builder, right, left);

  // if left is an identifier, set
  // if (l->type == FL_AST_LIT_IDENTIFIER) {
  //  ast_t* id = ast_search_id_decl(node, l->identifier.string);
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
  LLVMValueRef lhs = cg_ast_loaded("load_lhs", l, FL_CODEGEN_PASSTHROUGH);

  LLVMValueRef rhs = cg_ast_loaded("load_rhs", r, FL_CODEGEN_PASSTHROUGH);

  log_verbose("using binop %d", node->binop.operator);

  // common operations that works with any type
  switch (node->binop.operator) {
  case '&':
    return LLVMBuildAnd(builder, lhs, rhs, "and");
  case '|':
    return LLVMBuildOr(builder, lhs, rhs, "or");
  case '^':
    return LLVMBuildXor(builder, lhs, rhs, "xor");
  case TK_SHR:
    // returns the first operand shifted to the right a specified number of bits
    // with sign extension
    return LLVMBuildAShr(builder, lhs, rhs, "ashr");
  // TODO
  // logical shift right - lshr
  // return LLVMBuildLShr(builder, lhs, rhs, "lshr");
  // signed
  case TK_SHL:
    // returns the first operand shifted to the left a specified number of bits.
    return LLVMBuildShl(builder, lhs, rhs, "shl");
  default: {}
  }

  // left and right must have the same type, so pick one
  // current node could have other type
  bool use_fp = ty_is_fp(node->binop.left->ty_id);
  log_verbose("is fp? %d", use_fp);
  // Create different IR code depending on the operator.
  switch (node->binop.operator) {
  case TK_EQEQ: { // ==
    return use_fp ? LLVMBuildFCmp(builder, LLVMRealOEQ, lhs, rhs, "andand")
                  : LLVMBuildICmp(builder, LLVMIntEQ, lhs, rhs, "andand");
  }
  case TK_NE: { // !=
    return use_fp ? LLVMBuildFCmp(builder, LLVMRealONE, lhs, rhs, "andand")
                  : LLVMBuildICmp(builder, LLVMIntNE, lhs, rhs, "andand");
  }
  case '>': { // >
    // TODO LLVMIntUGT
    // TODO LLVMRealUGT

    return use_fp ? LLVMBuildFCmp(builder, LLVMRealOGT, lhs, rhs, "andand")
                  : LLVMBuildICmp(builder, LLVMIntSGT, lhs, rhs, "andand");
  }
  case TK_GE: { // >=
    // TODO LLVMIntUGE
    // TODO LLVMRealUGE

    return use_fp ? LLVMBuildFCmp(builder, LLVMRealOGE, lhs, rhs, "andand")
                  : LLVMBuildICmp(builder, LLVMIntSGE, lhs, rhs, "andand");
  }
  case '<': { // <
    // TODO LLVMIntULT
    // TODO LLVMRealULT

    return use_fp ? LLVMBuildFCmp(builder, LLVMRealOLT, lhs, rhs, "ltf")
                  : LLVMBuildICmp(builder, LLVMIntSLT, lhs, rhs, "lti");
  }
  case TK_LE: { // <=
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

  case '+': {
    return use_fp ? LLVMBuildFAdd(builder, lhs, rhs, "add")
                  : LLVMBuildAdd(builder, lhs, rhs, "addi");
  }
  case '-': {
    return use_fp ? LLVMBuildFSub(builder, lhs, rhs, "sub")
                  : LLVMBuildSub(builder, lhs, rhs, "subi");
  }
  case '*': {
    return use_fp ? LLVMBuildFMul(builder, lhs, rhs, "mul")
                  : LLVMBuildMul(builder, lhs, rhs, "muli");
  }
  case '/': {
    // signed vs unsigned
    return use_fp ? LLVMBuildFDiv(builder, lhs, rhs, "div")
                  : LLVMBuildSDiv(builder, lhs, rhs, "divi");
  }
  case '%': {
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
  log_debug("cg_function %s", node->func.uid
                                  ? node->func.uid->value
                                  : node->func.id->identifier.string->value);

  if (!node->func.ret_type) {
    log_error("function has no return type");
  }

  // TODO use type info should be faster
  size_t i = 0;
  ast_t* params = node->func.params;
  ast_t* tmp;

  LLVMTypeRef param_types[params->list.count];

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
                       params->list.count, node->func.varargs);

  LLVMValueRef func = LLVMAddFunction(module, node->func.uid->value, ret_type);
  LLVMSetFunctionCallConv(func, LLVMCCallConv);
  LLVMSetLinkage(func, LLVMExternalLinkage);

  LLVMBasicBlockRef block = 0;
  if (node->func.body) {
    block = LLVMAppendBasicBlock(func, "function-block");
    LLVMPositionBuilderAtEnd(builder, block); // new block
  }

  i = 0;
  if (node->func.params) {
    while ((tmp = params->list.elements[i]) != 0) {
      log_verbose("set name [%zu] '%s'", i,
                  tmp->param.id->identifier.string->value);
      LLVMValueRef param = LLVMGetParam(func, i);
      LLVMSetValueName(param, tmp->param.id->identifier.string->value);

      if (block) {
        LLVMValueRef ref = LLVMBuildAlloca(builder, LLVMTypeOf(param), "arg");
        LLVMBuildStore(builder, param, ref);
        tmp->param.alloca = (void*)ref;
      } else {
        tmp->param.alloca = (void*)param;
      }

      ++i;
    }
  }

  if (block) {
    // save and retore parent
    LLVMValueRef _parent = *parent;
    *parent = func;
    LLVMPositionBuilderAtEnd(builder, *current_block);
    cg_do_block(block, 0, node->func.body, FL_CODEGEN_PASSTHROUGH);
    *parent = _parent;
  }

  node->func.cg_decl = func;

  return func;
}

LLVMValueRef cg_return(FL_CODEGEN_HEADER) {
  log_debug("cg_return");

  LLVMValueRef argument =
      cg_ast_loaded("lret", node->ret.argument, FL_CODEGEN_PASSTHROUGH);
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
      log_verbose("argument idx [%zu]", i);
      arguments[i++] = cg_ast_loaded("arg_load", tmp, FL_CODEGEN_PASSTHROUGH);
    }

    return LLVMBuildCall(builder, fn, arguments, node->call.narguments, "");
  }

  LLVMValueRef empty_args[1];
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

LLVMValueRef cg_runary(FL_CODEGEN_HEADER) {
  LLVMValueRef element = cg_ast_loaded("load_lunary", node->runary.element,
                                       FL_CODEGEN_PASSTHROUGH);

  switch (node->runary.operator) {
  case TK_PLUSPLUS:
  case TK_MINUSMINUS: {
    LLVMTypeRef type = LLVMTypeOf(element);
    LLVMValueRef ret = LLVMBuildBinOp(
        builder, ty_is_fp(node->ty_id) ? LLVMFAdd : LLVMAdd, element,
        LLVMConstInt(type, node->lunary.operator== TK_PLUSPLUS ? 1 : -1, false),
        "radd");
    ast_t* el = node->lunary.element;
    if (el->type == FL_AST_LIT_IDENTIFIER) {
      cg_utils_store(el, ret, builder);
    }

    return element;
  }
  default: {}
  }
  log_error("runary not handled %d", node->lunary.operator);
  return 0;
}

LLVMValueRef cg_lunary(FL_CODEGEN_HEADER) {
  log_debug("cg_lunary");

  // raw: DO NOT LOAD!
  if (node->lunary.operator== '&') {
    return cg_ast(node->lunary.element, FL_CODEGEN_PASSTHROUGH);
  }

  LLVMValueRef element = cg_ast_loaded("load_lunary", node->lunary.element,
                                       FL_CODEGEN_PASSTHROUGH);

  switch (node->lunary.operator) {
  case '-':
    return LLVMBuildNeg(builder, element, "negate");
  case '!':
    return LLVMBuildNot(builder, element, "not");
  case TK_PLUSPLUS:
  case TK_MINUSMINUS: {
    LLVMTypeRef type = LLVMTypeOf(element);
    LLVMValueRef ret = LLVMBuildBinOp(
        builder, ty_is_fp(node->ty_id) ? LLVMFAdd : LLVMAdd, element,
        LLVMConstInt(type, node->lunary.operator== TK_PLUSPLUS ? 1 : -1, false),
        "ladd");
    ast_t* el = node->lunary.element;
    if (el->type == FL_AST_LIT_IDENTIFIER) {
      cg_utils_store(el, ret, builder);
    }

    return ret;
  }
  default: {}
  }
  log_error("lunary not handled %d", node->lunary.operator);
  return 0;
  /*
  case '+':
  case TK_TILDE:
  case TK_DELETE:
  */
}

LLVMValueRef cg_if(FL_CODEGEN_HEADER) {
  log_debug("(codegen) cg_if");
  bool has_else = node->if_stmt.alternate > 0;

  // end at the end
  LLVMBasicBlockRef end_block = LLVMAppendBasicBlock(*parent, "if-end");
  LLVMMoveBasicBlockAfter(end_block, *current_block);

  // else in the middle
  LLVMBasicBlockRef if_else_block;
  if (has_else) {
    if_else_block = LLVMAppendBasicBlock(*parent, "if-false");
    LLVMMoveBasicBlockAfter(if_else_block, *current_block);
  }

  // then at the top
  LLVMBasicBlockRef if_then_block = LLVMAppendBasicBlock(*parent, "if-true");
  LLVMMoveBasicBlockAfter(if_then_block, *current_block);

  // test expression
  ast_t* test_node = node->if_stmt.test;
  LLVMValueRef test =
      cg_ast_loaded("load_test", test_node, FL_CODEGEN_PASSTHROUGH);

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

  LLVMBasicBlockRef main_block = *current_block;

  // blocks are backwards!
  LLVMBasicBlockRef pre_cond = 0;
  LLVMBasicBlockRef post_cond = 0;
  LLVMBasicBlockRef update = 0;
  LLVMBasicBlockRef start = LLVMAppendBasicBlock(*parent, "loop-start");
  LLVMBasicBlockRef block = LLVMAppendBasicBlock(*parent, "loop-block");
  LLVMBasicBlockRef end_block = LLVMAppendBasicBlock(*parent, "loop-end");

  LLVMBuildBr(builder, start); // current-block -> start
  // initialization at start
  if (node->loop.init) {
    log_debug("loop initialization");
    cg_do_block(start, 0, node->loop.init, FL_CODEGEN_PASSTHROUGH);
  }

  if (node->loop.pre_cond) {
    pre_cond = LLVMAppendBasicBlock(*parent, "loop-pre_cond");

    LLVMPositionBuilderAtEnd(builder, start);
    LLVMBuildBr(builder, pre_cond);
    LLVMPositionBuilderAtEnd(builder, main_block);

    // test expression

    LLVMPositionBuilderAtEnd(builder, pre_cond);
    LLVMValueRef pre_cond_test =
        cg_ast(node->loop.pre_cond, FL_CODEGEN_PASSTHROUGH);
    LLVMBuildCondBr(builder, pre_cond_test, block, end_block);
    LLVMPositionBuilderAtEnd(builder, main_block);
  } else {
    LLVMPositionBuilderAtEnd(builder, start);
    LLVMBuildBr(builder, block);
    LLVMPositionBuilderAtEnd(builder, main_block);
  }

  if (node->loop.update) {
    log_debug("loop update");
    assert(pre_cond != 0);
    update = LLVMAppendBasicBlock(*parent, "loop-update");
    cg_do_block(update, pre_cond, node->loop.update, FL_CODEGEN_PASSTHROUGH);
  }

  if (node->loop.post_cond) {
    post_cond = LLVMAppendBasicBlock(*parent, "loop-post_cond");

    // test expression

    LLVMPositionBuilderAtEnd(builder, post_cond);
    LLVMValueRef post_cond_test =
        cg_ast(node->loop.post_cond, FL_CODEGEN_PASSTHROUGH);
    LLVMBuildCondBr(builder, post_cond_test, block, end_block);
    LLVMPositionBuilderAtEnd(builder, main_block);
  }

  log_debug("loop block");
  cg_do_block(block, update ? update
                            : (pre_cond ? pre_cond
                                        : (post_cond ? post_cond : end_block)),
              node->loop.block, FL_CODEGEN_PASSTHROUGH);

  LLVMMoveBasicBlockAfter(end_block, main_block);
  if (update) {
    LLVMMoveBasicBlockAfter(update, main_block);
  }
  LLVMMoveBasicBlockAfter(block, main_block);
  if (pre_cond) {
    LLVMMoveBasicBlockAfter(pre_cond, main_block);
  }
  LLVMMoveBasicBlockAfter(start, main_block);

  LLVMPositionBuilderAtEnd(builder, end_block);
  *current_block = end_block;

  return 0;
}

LLVMValueRef cg_left_identifier(FL_CODEGEN_HEADER) {
  log_debug("identifier");
  ast_t* decl = ast_search_id_decl(node, node->identifier.string);

  if (!decl) {
    log_error("identifier not found: '%s'", node->identifier.string->value);
  }

  if (decl->type == FL_AST_DTOR_VAR) {
    return (LLVMValueRef)decl->var.alloca;
  }
  if (decl->type == FL_AST_PARAMETER) {
    return (LLVMValueRef)decl->param.alloca;
  }

  if (decl->type == FL_AST_DECL_FUNCTION) {
    return (LLVMValueRef)decl->func.cg_decl;
  }

  log_error("unhandled identifier decl %d", decl->type);

  return 0;
}

LLVMValueRef cg_right_identifier(FL_CODEGEN_HEADER) {
  ast_t* decl = node->identifier.decl;

  if (!decl) {
    ast_dump(node->parent);
    log_error("identifier not found: '%s'", node->identifier.string->value);
  }

  if (decl->type == FL_AST_DTOR_VAR) {
    return (LLVMValueRef)decl->var.alloca;
  }

  if (decl->type == FL_AST_PARAMETER) {
    return (LLVMValueRef)decl->param.alloca;
  }

  if (decl->type == FL_AST_DECL_FUNCTION) {
    return (LLVMValueRef)decl->func.cg_decl;
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
    LLVMValueRef left =
        cg_ast_loaded("load_ptr", node->member.left, FL_CODEGEN_PASSTHROUGH);
    LLVMValueRef index[1];

    if (node->member.expression) {
      index[0] = cg_ast_loaded("left_expr_loaded", node->member.property,
                               FL_CODEGEN_PASSTHROUGH);
    } else {
      index[0] = cg_ast(node->member.property, FL_CODEGEN_PASSTHROUGH);
    }

    return LLVMBuildGEP(builder, left, index, 1, "ptr");
    // return LLVMBuildInBoundsGEP(builder, left, index, 1, "");
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

LLVMValueRef cg_sizeof(FL_CODEGEN_HEADER) {
  return LLVMSizeOf(cg_get_type(node->sof.type, context));
}
