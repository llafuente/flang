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

LLVMExecutionEngineRef cg_jit(LLVMModuleRef M) {
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
