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

extern int log_debug_level;

// 0 - error
// 1 - warning
// 2 - info
// 3 - debug
// 4 - verbose
// 5 - silly
#define dbg(level, ...)                                                        \
  if (log_debug_level >= level) {                                              \
    char buf[] = __FILE__;                                                     \
    fprintf(stderr, "%20s:%d ", basename(buf), __LINE__);                        \
    fprintf(stderr, __VA_ARGS__);                                              \
    fprintf(stderr, "\n");                                                     \
    if (level == 0) {                                                          \
      void* array[10];                                                         \
      size_t size;                                                             \
      char** strings;                                                          \
      size_t i;                                                                \
                                                                               \
      size = backtrace(array, 10);                                             \
      strings = backtrace_symbols(array, size);                                \
                                                                               \
      fprintf(stderr, "Obtained %zd stack frames.\n", size);                   \
                                                                               \
      for (i = 0; i < size; i++) {                                             \
        fprintf(stderr, "%s\n", strings[i]);                                   \
      }                                                                        \
                                                                               \
      free(strings);                                                           \
                                                                               \
      exit(1);                                                                 \
    }                                                                          \
  }

#define log_error(...) dbg(0, __VA_ARGS__)
#define log_warning(...) dbg(1, __VA_ARGS__)
#define log_info(...) dbg(2, __VA_ARGS__)
#define log_debug(...) dbg(3, __VA_ARGS__)
#define log_verbose(...) dbg(4, __VA_ARGS__)
#define log_silly(...) dbg(5, __VA_ARGS__)
