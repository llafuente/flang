#include <inttypes.h>
#include "stringc.h"
#include <pthread.h>
#include "uv.h"
#include <assert.h>

#define MAX_PATH 1024

#define public extern
#define globvar static
#define var
#define cstr char*

// hints
#define virtual
#define wrapper

#define null 0
#define true 1
#define false 0
//#define bool int8_t
#define i8 int8_t
#define i16 int16_t
#define i32 int32_t
#define i64 int64_t
#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t
#define f32 float
#define f64 double

typedef struct {
  string* cwd;
} process_t;
globvar process_t* process;

/// DECL

/// END OF DECL

void bootstrap() {
  process = malloc(sizeof(process_t));
  process->cwd = st_new(MAX_PATH, st_enc_utf8);
  size_t size = 1027;
  int err = uv_cwd(process->cwd->value, &size);
  assert(err == 0);
}

int run();

int main() {
  bootstrap();

  run();

  st_delete(&process->cwd);
  free(process);
  return 0;
}

/// IMPL

/// END OF IMPL
