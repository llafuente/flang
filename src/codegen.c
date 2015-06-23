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

LLVMExecutionEngineRef fl_codegen_jit(LLVMModuleRef M) {
  LLVMExecutionEngineRef MCJIT;
  char* err_str;
  struct LLVMMCJITCompilerOptions Options;

  LLVMInitializeMCJITCompilerOptions(&Options, sizeof(Options));
  Options.OptLevel = 0;
  Options.CodeModel = LLVMCodeModelDefault;
  // NoFramePointerElim - This flag is enabled when the -disable-fp-elim is
  // specified on the command line.  If the target supports the frame pointer
  // elimination optimization, this option should disable it.
  Options.NoFramePointerElim = true;
  // EnableFastISel - This flag enables fast-path instruction selection
  // which trades away generated code quality in favor of reducing
  // compile time.
  Options.EnableFastISel = true;
  Options.MCJMM = 0;

  if (LLVMCreateMCJITCompilerForModule(&MCJIT, M, &Options, sizeof(Options),
                                       &err_str)) {
    fputs(err_str, stderr);
    LLVMDisposeMessage(err_str);
  }

  return MCJIT;
}

void fl_interpreter(LLVMModuleRef module) {
  char* err;

  LLVMExecutionEngineRef interp;
  LLVMInitializeNativeTarget();
  LLVMLinkInInterpreter(); // "Interpreter has not been linked"

  if (LLVMCreateInterpreterForModule(&interp, module, &err) != 0) {
    fputs("LLVMCreateInterpreterForModule\n", stderr);
    fputs(err, stderr);
  }

  LLVMDisposeMessage(err);

  /*
    LLVMGenericValueRef main_args[] = {
        LLVMCreateGenericValueOfPointer(0),
        LLVMCreateGenericValueOfInt(LLVMInt32Type(), 0, false)};
  */
  LLVMGenericValueRef res =
      LLVMRunFunction(interp, LLVMGetNamedFunction(module, "main"), 0, 0);

  LLVMDisposeExecutionEngine(interp);
  LLVMDisposeModule(module);
}

bool fl_to_bitcode(LLVMModuleRef module, const char* filename) {
  // Write out bitcode to file
  if (LLVMWriteBitcodeToFile(module, filename) != 0) {
    fprintf(stderr, "error writing bitcode to file '%s'\n", filename);
    return false;
  }
  return true;
}

// TODO check file, return false!
bool fl_to_ir(LLVMModuleRef module, const char* filename) {
  char* irstr = LLVMPrintModuleToString(module);
  FILE* f = fopen(filename, "w");
  fputs(irstr, f);
  fclose(f);
  LLVMDisposeMessage(irstr);

  return true;
}

LLVMModuleRef fl_codegen(fl_ast_t* root, char* module_name) {
  char* err;
  LLVMContextRef context = LLVMGetGlobalContext();
  LLVMModuleRef module =
      // LLVMModuleCreateWithNameInContext(!module_name ? "main" : module_name,
      // context);
      LLVMModuleCreateWithName(!module_name ? "module" : module_name);
  // LLVMBuilderRef builder = LLVMCreateBuilderInContext(context);
  LLVMBuilderRef builder = LLVMCreateBuilder();

  /*
    LLVMExecutionEngineRef jit = fl_codegen_jit();

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
  LLVMValueRef main = LLVMAddFunction(
      module, "main", LLVMFunctionType(LLVMInt32Type(), 0, 0, 0));
  LLVMSetFunctionCallConv(main, LLVMCCallConv);
  LLVMBasicBlockRef current_block = LLVMAppendBasicBlock(main, "main-entry");

  LLVMPositionBuilderAtEnd(builder, current_block);

  // node must have parent, because we need to search backwards
  fl_ast_parent(root);
  fl_codegen_ast(root, FL_CODEGEN_PASSTHROUGH);

  LLVMBuildRet(builder, LLVMConstInt(LLVMInt32Type(), 0, false));
  // optimize main
  // LLVMRunFunctionPassManager(pass_manager, main);

  // LLVMAbortProcessAction?
  LLVMVerifyModule(module, LLVMPrintMessageAction, &err);
  puts("LLVMVerifyModule\n");
  puts(err);
  LLVMDisposeMessage(err);

  char* irstr = LLVMPrintModuleToString(module);
  puts("LLVMPrintModuleToString\n");
  puts(irstr);
  LLVMDisposeMessage(irstr);

  // LLVMDumpModule(module);
  // LLVMDisposePassManager(pass_manager);
  LLVMDisposeBuilder(builder);

  return module;
}

LLVMValueRef fl_codegen_ast(FL_CODEGEN_HEADER) {

  switch (node->type) {
  case FL_AST_PROGRAM:
    cg_print("(codegen) program\n");

    if (node->program.core) {
      cg_print("(codegen) generating core\n");
      fl_codegen_ast(node->program.core, FL_CODEGEN_PASSTHROUGH);
    }

    return fl_codegen_ast(node->program.body, FL_CODEGEN_PASSTHROUGH);
  case FL_AST_BLOCK: {
    cg_print("(codegen) block\n");
    size_t i = 0;
    fl_ast_t* tmp;

    while ((tmp = node->block.body[i++])) {
      cg_print("(codegen) block %zu\n", i);
      fl_codegen_ast(tmp, FL_CODEGEN_PASSTHROUGH);
    }

    return 0;
  }
  case FL_AST_EXPR_ASSIGNAMENT:
    cg_print("(codegen) assignament\n");
    return fl_codegen_assignament(FL_CODEGEN_HEADER_SEND);
  case FL_AST_EXPR_BINOP:
    cg_print("(codegen) binary operator\n");
    return fl_codegen_binop(FL_CODEGEN_HEADER_SEND);
  case FL_AST_EXPR_CALL:
    cg_print("(codegen) expression call\n");
    return fl_codegen_expr_call(FL_CODEGEN_HEADER_SEND);
  case FL_AST_LIT_NUMERIC:
    cg_print("(codegen) number\n");
    return fl_codegen_lit_number(FL_CODEGEN_HEADER_SEND);
  case FL_AST_LIT_BOOLEAN:
    return fl_codegen_lit_boolean(FL_CODEGEN_HEADER_SEND);
  case FL_AST_LIT_STRING:
    cg_print("(codegen) string\n");
    return fl_codegen_lit_string(FL_CODEGEN_HEADER_SEND);
  case FL_AST_LIT_IDENTIFIER: {
    cg_print("(codegen) identifier\n");
    fl_ast_t* id = fl_ast_search_decl_var(node, node->identifier.string);

    if (!id) {
      cg_error("(codegen) identifier not found: %s\n",
               node->identifier.string->value);
    }

    if (id->var.dirty) {
      // load it!
      // TODO this is a nice hack but need to be refactored
      // TODO mark variable as dirty
      // TODO review, seams to be always necessary
      // if (fl_ast_is_pointer(left_ast)) {
      //}

      // TODO this is the cause of some nasty bug!
      id->var.current = (void*)LLVMBuildLoad(builder, id->codegen, "load");
    }

    return (LLVMValueRef)id->var.current;
  }
  case FL_AST_DTOR_VAR:
    cg_print("(codegen) dtor\n");
    return fl_codegen_dtor_var(FL_CODEGEN_HEADER_SEND);
  case FL_AST_DECL_FUNCTION:
    cg_print("(codegen) function\n");
    return fl_codegen_function(FL_CODEGEN_HEADER_SEND);
  case FL_AST_STMT_RETURN:
    cg_print("(codegen) return\n");
    return fl_codegen_return(FL_CODEGEN_HEADER_SEND);
  case FL_AST_CAST:
    cg_print("(codegen) cast\n");
    return fl_codegen_cast(FL_CODEGEN_HEADER_SEND);
  case FL_AST_EXPR_LUNARY:
    cg_print("(codegen) lunary\n");
    return fl_codegen_lunary(FL_CODEGEN_HEADER_SEND);
  default: {}
    // cg_error("(codegen) ast->type not handled %d\n", node->type);
  }
  return 0;
}

LLVMValueRef fl_codegen_cast(FL_CODEGEN_HEADER) {
  // check if rigt side is a constant
  if (node->cast.right->type == FL_AST_LIT_NUMERIC) {
    printf("(codegen) cast override to %zu\n", node->cast.to->ty.id);
    node->cast.right->numeric.ty_id = node->cast.to->ty.id;

    return fl_codegen_ast(node->cast.right, FL_CODEGEN_PASSTHROUGH);
  }

  LLVMValueRef right = fl_codegen_ast(node->cast.right, FL_CODEGEN_PASSTHROUGH);

  printf("(codegen) do cast [%p]\n", right);
  return fl_codegen_cast_op(builder, fl_ast_get_typeid(node->cast.right),
                            fl_ast_get_typeid(node->cast.to), right);
}

LLVMValueRef fl_codegen_lit_number(FL_CODEGEN_HEADER) {
  // get parent type, to know what type should i be.
  size_t ty = fl_ast_get_typeid(node->parent);
  // size_t ty = node->numeric.ty_id;
  fl_type_t t = fl_type_table[ty];

  if (t.number.fp) {
    return LLVMConstReal(fl_codegen_get_typeid(ty), node->numeric.value);
  }

  return LLVMConstInt(fl_codegen_get_typeid(ty), node->numeric.value,
                      t.number.sign);
}

LLVMValueRef fl_codegen_lit_boolean(FL_CODEGEN_HEADER) {
  fl_type_t t = fl_type_table[2];

  return LLVMConstInt(fl_codegen_get_typeid(2), node->boolean.value, false);
}

LLVMValueRef fl_codegen_lit_string(FL_CODEGEN_HEADER) {
  return LLVMBuildGlobalStringPtr(builder, node->string.value->value,
                                  "string_val" // TODO must be unique!
                                  );
}

LLVMValueRef fl_codegen_assignament(FL_CODEGEN_HEADER) {
  fprintf(stderr, "(codegen) assignament\n");

  fl_ast_t* left_decl =
      fl_ast_search_decl_var(node, node->assignament.left->identifier.string);

  if (!left_decl) {
    cg_error("(codegen) lhs cannot be fetch\n");
  }

  LLVMValueRef left = (LLVMValueRef)left_decl->codegen;

  // LLVMDoubleTypeInContext(context);
  LLVMValueRef right =
      fl_codegen_ast(node->assignament.right, FL_CODEGEN_PASSTHROUGH);

  cg_print("(codegen) %p = %p\n", left, right);

  // TODO binop casting!
  // size_t ltype = fl_ast_get_typeid(left);
  // size_t rtype = fl_ast_get_typeid(right);
  // size_t rtype = fl_ast_get_typeid(right);

  LLVMValueRef assign = LLVMBuildStore(builder, right, left);
  left_decl->var.dirty = true;

  // maybe return the lhs load!?

  return assign;
}

LLVMValueRef fl_codegen_binop(FL_CODEGEN_HEADER) {
  fprintf(stderr, "(codegen) binop\n");

  // retrieve left/right side
  LLVMValueRef lhs = fl_codegen_ast(node->binop.left, FL_CODEGEN_PASSTHROUGH);
  LLVMValueRef rhs = fl_codegen_ast(node->binop.right, FL_CODEGEN_PASSTHROUGH);

  if (lhs == 0 || rhs == 0) {
    fprintf(stderr, "something it not right!");
    return 0;
  }
  printf("usgin binop %d\n", node->binop.operator);

  // Create different IR code depending on the operator.
  switch (node->binop.operator) {
  case FL_TK_PLUS: {
    return LLVMBuildFAdd(builder, lhs, rhs, "add");
  }
  case FL_TK_MINUS: {
    return LLVMBuildFSub(builder, lhs, rhs, "sub");
  }
  case FL_TK_ASTERISK: {
    return LLVMBuildFMul(builder, lhs, rhs, "mul");
  }
  case FL_TK_SLASH: {
    return LLVMBuildFDiv(builder, lhs, rhs, "div");
  }
  case FL_TK_MOD: {
    return LLVMBuildFRem(builder, lhs, rhs, "mod");
  }
  case FL_TK_AND: {
    return LLVMBuildAnd(builder, lhs, rhs, "and");
  }
  case FL_TK_OR: {
    return LLVMBuildOr(builder, lhs, rhs, "or");
  }
  case FL_TK_CARET: {
    return LLVMBuildXor(builder, lhs, rhs, "xor");
  }

  default:
    fprintf(stderr, "(codegen) binop not supported: %d\n",
            node->binop.operator);
  }

  return 0;
}

// TODO manage type
LLVMValueRef fl_codegen_dtor_var(FL_CODEGEN_HEADER) {

  if (!node->var.type) {
    fputs("variable has no type cannot be generated\n", stderr);
    return 0;
  }

  LLVMValueRef ref =
      LLVMBuildAlloca(builder, fl_codegen_get_type(node->var.type),
                      node->var.id->identifier.string->value);
  node->codegen = (void*)ref;

  return ref;
}

LLVMValueRef fl_codegen_function(FL_CODEGEN_HEADER) {

  if (!node->func.ret_type) {
    fprintf(stderr, "No return type?");
    exit(1);
  }

  LLVMTypeRef param_types[node->func.nparams];

  // TODO manage type
  size_t i = 0;
  fl_ast_t* tmp;

  if (node->func.params) {
    while ((tmp = node->func.params[i]) != 0) {
      if (!tmp->param.type) {
        cg_error("(codegen) Parameter %zu don't have type.\n", i);
      }

      cg_print("(codegen) parameter %zu of type %d\n", i, tmp->param.type);
      param_types[i++] = fl_codegen_get_type(tmp->param.type);
    }
  }
  // TODO manage return type
  LLVMTypeRef ret_type =
      LLVMFunctionType(fl_codegen_get_type(node->func.ret_type), param_types,
                       node->func.nparams, node->func.varargs);

  LLVMValueRef func = LLVMAddFunction(
      module, node->func.id->identifier.string->value, ret_type);
  LLVMSetFunctionCallConv(func, LLVMCCallConv);
  LLVMSetLinkage(func, LLVMExternalLinkage);

  i = 0;
  if (node->func.params) {
    while ((tmp = node->func.params[i]) != 0) {
      printf("set name %zu\n", i);
      LLVMValueRef param = LLVMGetParam(func, i);
      LLVMSetValueName(param, tmp->param.id->identifier.string->value);
      tmp->codegen = (void*)param;
      ++i;
    }
  }

  if (node->func.body) {
    LLVMBasicBlockRef block = LLVMAppendBasicBlock(func, "function-block");
    LLVMPositionBuilderAtEnd(builder, block);

    fl_codegen_ast(node->func.body, FL_CODEGEN_PASSTHROUGH);

    LLVMPositionBuilderAtEnd(builder, current_block);
  }

  return func;
}
LLVMValueRef fl_codegen_return(FL_CODEGEN_HEADER) {
  LLVMValueRef argument =
      fl_codegen_ast(node->ret.argument, FL_CODEGEN_PASSTHROUGH);
  LLVMBuildRet(builder, argument);

  return 0;
}

LLVMValueRef fl_codegen_expr_call(FL_CODEGEN_HEADER) {
  LLVMValueRef arguments[node->call.narguments];
  LLVMValueRef value;

  size_t i = 0;
  fl_ast_t* tmp;

  if (node->call.arguments) {
    while ((tmp = node->call.arguments[i]) != 0) {
      printf("argument! %zu\n", i);
      value = fl_codegen_ast(tmp, FL_CODEGEN_PASSTHROUGH);
      if (!value) {
        fprintf(stderr, "argument fail! %zu\n", i);
        exit(1);
      }
      arguments[i++] = value;
    }
  }
  LLVMValueRef fn =
      LLVMGetNamedFunction(module, node->call.callee->identifier.string->value);
  if (!fn) {
    fprintf(stderr, "function %s not found\n",
            node->call.callee->identifier.string->value);
    exit(1);
  }

  return LLVMBuildCall(builder, fn, arguments, node->call.narguments, "call");

  /*
  LLVMValueRef argument =
      fl_codegen_ast(node->ret.argument, FL_CODEGEN_PASSTHROUGH);
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
LLVMValueRef fl_codegen_lunary(FL_CODEGEN_HEADER) {
  cg_print("(codegen) negate\n");
  return LLVMBuildNeg(
      builder, fl_codegen_ast(node->lunary.element, FL_CODEGEN_PASSTHROUGH),
      "negate");
}
