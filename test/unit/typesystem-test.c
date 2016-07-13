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

#include "flang/common.h"
#include "flang/libts.h"
#include "../tasks.h"

TASK_IMPL(typesystem) {

  ts_init();

  size_t t = ts_promote_typeid(TS_I8, TS_I16);
  ASSERT(t == TS_I16, "TS_I8 + TS_I16 => TS_I16");

  t = ts_promote_typeid(TS_I8, TS_U8);
  ASSERT(t == TS_I8, "TS_I8 + TS_U8 => TS_I8");

  t = ts_promote_typeid(TS_I8, TS_I8);
  ASSERT(t == TS_I8, "TS_I8 + TS_I8 => TS_I8");

  t = ts_promote_typeid(TS_F32, TS_I8);
  ASSERT(t == TS_F32, "TS_I8 + TS_F32 => TS_F32");

  t = ts_promote_typeid(TS_BOOL, TS_F32);
  ASSERT(t == TS_F32, "TS_BOOL + TS_F32 => TS_F32");

  t = ts_promote_typeid(TS_I32, TS_BOOL);
  ASSERT(t == TS_I32, "TS_BOOL + TS_I32 => TS_I32");

  t = ts_promote_typeid(TS_BOOL, TS_I32);
  ASSERT(t == TS_I32, "TS_BOOL + TS_I32 => TS_I32");

  ASSERT(!ts_castable(TS_I64, TS_I32), "demotion not allowed");

  ts_exit();

  return 0;
}
