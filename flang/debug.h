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

//-
//- DEBUG STAFF
//-

#include <libgen.h>

extern void __sanitizer_print_stack_trace();
extern int log_debug_level;

#define fl_assert(chk)                                                         \
  if (!(chk)) {                                                                \
    fprintf(stderr, "%s: Assertion failed at %s:%d @ %s\n", #chk, __FILE__,    \
            __LINE__, __FUNCTION__);                                           \
    __sanitizer_print_stack_trace();                                           \
    exit(5);                                                                   \
  }

#define fl_fatal_error(fmt, ...)                                               \
  fprintf(stderr, "Faltal error at %s:%d @ %s\n", __FILE__, __LINE__,          \
          __FUNCTION__);                                                       \
  fprintf(stderr, fmt, __VA_ARGS__);                                           \
  __sanitizer_print_stack_trace();                                             \
  exit(5);

// 0 - error
// 1 - warning
// 2 - info
// 3 - debug
// 4 - verbose
// 5 - silly
#define dbg(trace, level, ...)                                                 \
  do {                                                                         \
    if (log_debug_level >= level) {                                            \
      char buf[] = __FILE__;                                                   \
      if (trace) {                                                             \
        fprintf(stderr, "%20s:%3d[%s] ", basename(buf), __LINE__,              \
                __FUNCTION__);                                                 \
      }                                                                        \
      fprintf(stderr, __VA_ARGS__);                                            \
      if (trace) {                                                             \
        fprintf(stderr, "\n");                                                 \
      }                                                                        \
      if (level == 0) {                                                        \
        __sanitizer_print_stack_trace();                                       \
        exit(6);                                                               \
      }                                                                        \
    }                                                                          \
  } while (false)

#define log_error(...) dbg(true, 0, __VA_ARGS__)
#define log_warning(...) dbg(true, 1, __VA_ARGS__)
#define log_info(...) dbg(true, 2, __VA_ARGS__)
#define log_debug(...) dbg(true, 3, __VA_ARGS__)
#define log_verbose(...) dbg(true, 4, __VA_ARGS__)
#define log_silly(...) dbg(true, 5, __VA_ARGS__)

#define log_error2(...) dbg(false, 0, __VA_ARGS__)
#define log_warning2(...) dbg(false, 1, __VA_ARGS__)
#define log_info2(...) dbg(false, 2, __VA_ARGS__)
#define log_debug2(...) dbg(false, 3, __VA_ARGS__)
#define log_verbose2(...) dbg(false, 4, __VA_ARGS__)
#define log_silly2(...) dbg(false, 5, __VA_ARGS__)
