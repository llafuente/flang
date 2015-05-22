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

int fl_codegen(fl_ast_t* root, char* module_name) {
  char* err;
  LLVMContextRef context = LLVMGetGlobalContext();
  LLVMModuleRef module =
      // LLVMModuleCreateWithNameInContext(!module_name ? "main" : module_name,
      // context);
      LLVMModuleCreateWithName(!module_name ? "main" : module_name);
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
  LLVMTypeRef printf_args[] = {LLVMPointerType(LLVMInt8Type(), 0)};
  LLVMValueRef printf =
      LLVMAddFunction(module, "printf",
                      LLVMFunctionType(LLVMInt32Type(), printf_args, 1, true));
  LLVMSetFunctionCallConv(printf, LLVMCCallConv);

  // create main
  LLVMTypeRef main_args[] = {LLVMPointerType(LLVMInt8Type(), 0),
                             LLVMInt32Type()};

  LLVMValueRef main = LLVMAddFunction(
      module, "main", LLVMFunctionType(LLVMInt32Type(), main_args, 2, 0));
  LLVMSetFunctionCallConv(main, LLVMCCallConv);
  LLVMBasicBlockRef entry = LLVMAppendBasicBlock(main, "main-entry");

  LLVMPositionBuilderAtEnd(builder, entry);

  // node must have parent, because we need to search backwards
  fl_ast_parent(root);
  fl_codegen_ast(root, builder, module, context);

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
  LLVMDisposeModule(module);

  return 0;
}

LLVMValueRef fl_codegen_ast(FL_CODEGEN_HEADER) {
  printf("node [%p] %d\n", node, node->type);

  switch (node->type) {
  case FL_AST_PROGRAM: {
    size_t i = 0;
    fl_ast_t* tmp;

    while ((tmp = node->program.body[i++])) {
      fl_codegen_ast(tmp, builder, module, context);
    }

    return 0;
  }
  case FL_AST_EXPR_ASSIGNAMENT:
    return fl_codegen_assignament(FL_CODEGEN_HEADER_SEND);
  case FL_AST_EXPR_BINOP:
    return fl_codegen_binop(FL_CODEGEN_HEADER_SEND);
  case FL_AST_LIT_NUMERIC:
    return fl_codegen_lit_number(FL_CODEGEN_HEADER_SEND);
  case FL_AST_LIT_IDENTIFIER:
    break;
  case FL_AST_DTOR_VAR:
    return fl_codegen_dtor_var(FL_CODEGEN_HEADER_SEND);
    break;
  default:
    fprintf(stderr, "(codegen) ast->type not handled %d\n", node->type);
  }
  return 0;
}

LLVMValueRef fl_codegen_lit_number(FL_CODEGEN_HEADER) {
  fprintf(stderr, "(codegen) number: %f\n", node->numeric.value);
  return LLVMConstReal(LLVMDoubleType(), node->numeric.value);
}

LLVMValueRef fl_codegen_assignament(FL_CODEGEN_HEADER) {
  fprintf(stderr, "(codegen) assignament\n");

  fl_ast_t* left_ast =
      fl_ast_search_decl_var(node, node->assignament.left->identifier.string);

  if (!left_ast) {
    fprintf(stderr, "(codegen) lhs cannot be fetch\n");
    return 0;
  }

  LLVMValueRef left = (LLVMValueRef)left_ast->codegen;

  // LLVMDoubleTypeInContext(context);
  LLVMValueRef right =
      fl_codegen_ast(node->assignament.right, builder, module, context);

  return LLVMBuildStore(builder, right, left);
}

LLVMValueRef fl_codegen_binop(FL_CODEGEN_HEADER) {
  fprintf(stderr, "(codegen) binop\n");

  // retrieve left/right side
  LLVMValueRef lhs = fl_codegen_ast(node->binop.left, builder, module, context);
  LLVMValueRef rhs =
      fl_codegen_ast(node->binop.right, builder, module, context);

  if (lhs == 0 || rhs == 0) {
    fprintf(stderr, "something it not right!");
    return 0;
  }
  printf("usgin binop %d\n", node->binop.operator);

  // Create different IR code depending on the operator.
  switch (node->binop.operator) {
  case FL_TK_PLUS: {
    printf("addtmp\n");
    return LLVMBuildFAdd(builder, lhs, rhs, "addtmp");
  }
  case FL_TK_MINUS: {
    return LLVMBuildFSub(builder, lhs, rhs, "subtmp");
  }
  case FL_TK_ASTERISK: {
    return LLVMBuildFMul(builder, lhs, rhs, "multmp");
  }
  case FL_TK_SLASH: {
    return LLVMBuildFDiv(builder, lhs, rhs, "divtmp");
  }
  default:
    fprintf(stderr, "(codegen) binop not supported: %d\n",
            node->binop.operator);
  }

  return 0;
}

LLVMTypeRef fl_codegen_get_type(fl_ast_t* node) {
  switch (node->idtype.of) {
  case FL_TK_BOOL:
    return LLVMIntType(1);
    break;
  case FL_TK_I8:
    return LLVMIntType(8);
    break;
  case FL_TK_I16:
    return LLVMIntType(16);
    break;
  case FL_TK_I32:
    return LLVMIntType(32);
    break;
  case FL_TK_I64:
    return LLVMIntType(64);
    break;
  case FL_TK_U8:
    return LLVMIntType(8);
    break;
  case FL_TK_U16:
    return LLVMIntType(16);
    break;
  case FL_TK_U32:
    return LLVMIntType(32);
    break;
  case FL_TK_U64:
    return LLVMIntType(64);
    break;
  // LLVMHalfType
  case FL_TK_F32:
    return LLVMFloatType();
    break;
  case FL_TK_F64:
    return LLVMDoubleType();
    break;
  case FL_TK_STRING:
    return LLVMDoubleType();
    break;
  }

  fputs("not defined type\n", stderr);
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
