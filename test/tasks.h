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

//
// assert macro + backtrace
//

#include "stringc.h"

#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <fcntl.h>
#include <time.h>

extern size_t st_assert_count;

#define STRINGIFY2(x) #x
#define STRINGIFY(x) STRINGIFY2(x)

#define PASTE2(a, b) a##b
#define PASTE(a, b) PASTE2(a, b)

#ifdef BENCHMARKING
#define ASSERT(comparison, name)                                               \
  if (comparison == false) {                                                   \
    printf("[%6zu]\x1B[31mFAIL\x1B[39m: %-32s [%s:%d]\n", ++st_assert_count,   \
           name, __FILE__, __LINE__);                                          \
    trace(stderr);                                                             \
    exit(1);                                                                   \
  }

#define ASSERT_STR(src, dst, enc)                                              \
  if (enc == st_enc_utf32be || enc == st_enc_utf32le) {                        \
    ASSERT(0 == wcscmp((const wchar_t*)src->value, (const wchar_t*)dst),       \
           "value");                                                           \
  } else {                                                                     \
    ASSERT(0 == strcmp((const char*)src->value, (const char*)dst), "value");   \
  }                                                                            \
  ASSERT(src->length == st_length((const char*)dst, enc), "length");           \
  ASSERT(src->used == st_capacity((const char*)dst, enc), "used");             \
  ASSERT(src->capacity >= src->used, "capacity");
#else
#define ASSERT(comparison, name)                                               \
  if (comparison) {                                                            \
    printf("[%6zu]\x1B[32mPASS\x1B[39m: %-32s [%s:%d]\n", ++st_assert_count,   \
           name, __FILE__, __LINE__);                                          \
  } else {                                                                     \
    printf("[%6zu]\x1B[31mFAIL\x1B[39m: %-32s [%s:%d]\n", ++st_assert_count,   \
           name, __FILE__, __LINE__);                                          \
    trace(stderr);                                                             \
    exit(1);                                                                   \
  }
#define ASSERT_STR(src, dst, enc)                                              \
  if (enc == st_enc_utf32be || enc == st_enc_utf32le) {                        \
    printf("# CHECK %s = utf32 L[%d]U[%d]C[%u]\n", STRINGIFY2(src),            \
           src->length, src->used, src->capacity);                             \
    printf("# AGAINST utf32 L[%d]C[%u]\n", st_length((const char*)dst, enc),   \
           st_capacity((const char*)dst, enc));                                \
    ASSERT(0 == wcscmp((const wchar_t*)src->value, (const wchar_t*)dst),       \
           "value");                                                           \
  } else {                                                                     \
    printf("# CHECK %s = '%s' L[%d]U[%d]C[%u]\n", STRINGIFY2(src), src->value, \
           src->length, src->used, src->capacity);                             \
    printf("# AGAINST '%s' L[%d]C[%u]\n", (char*)dst,                          \
           st_length((const char*)dst, enc),                                   \
           st_capacity((const char*)dst, enc));                                \
    ASSERT(0 == strcmp((const char*)src->value, (const char*)dst), "value");   \
  }                                                                            \
  ASSERT(src->length == st_length((const char*)dst, enc), "length");           \
  ASSERT(src->used == st_capacity((const char*)dst, enc), "used");             \
  ASSERT(src->capacity >= src->used, "capacity");

#endif

#define TASK_IMPL(name)                                                        \
  extern int run_task_##name(void);                                            \
  int run_task_##name(void)

#define TASK_RUN(name)                                                         \
  printf("\n\n################\n");                                            \
  printf("## %s\n", #name);                                                    \
  printf("################\n");                                                \
  extern int run_task_##name(void);                                            \
  run_task_##name();

#include <unistd.h>

// create a benchmark
// allways run times+3
// the first three times are to get everything hot
#define TASK_BENCHMARK(prefix, measurements, times, name)                      \
  extern int run_task_##name(void);                                            \
  {                                                                            \
    run_task_##name();                                                         \
    run_task_##name();                                                         \
    run_task_##name();                                                         \
    double lmeasurements[measurements];                                        \
    for (size_t m = 0; m < measurements; ++m) {                                \
      bench_start();                                                           \
      for (size_t i = 0; i < times; ++i) {                                     \
        run_task_##name();                                                     \
      }                                                                        \
      lmeasurements[m] = bench_end();                                          \
    }                                                                          \
    char filename[256];                                                        \
    sprintf(filename, "bench_data/%s-%s", prefix, #name);                      \
    FILE* fp = fopen(filename, "w+");                                          \
    if (!fp) {                                                                 \
      printf("cannot open file %s\n", filename);                               \
      exit(1);                                                                 \
    }                                                                          \
    char* date = bench_get_current_date();                                     \
    fprintf(fp, "# " #name ": %s\n", date);                                    \
    free(date);                                                                \
    for (size_t m = 0; m < measurements; ++m) {                                \
      fprintf(fp, "%f\n", lmeasurements[m]);                                   \
    }                                                                          \
    fclose(fp);                                                                \
  }

extern char* bench_get_current_date();

extern struct timespec* st_bench_time_start;
extern struct timespec* st_bench_time_end;

extern void bench_start();
extern double bench_end();

extern void trace(FILE* t);
