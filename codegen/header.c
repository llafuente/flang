#include <inttypes.h>
#include "stringc.h"

#define MAX_PATH 1024

#define public extern
#define globvar static
#define var

// hints
#define virtual
#define wrapper

#define null 0
#define true 1
#define false 0
#define bool int8_t
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
  process.cwd = st_new(MAX_PATH, st_enc_utf8)
  uv_err_t err;
  size_t size = sizeof(buffer_orig) / sizeof(buffer_orig[0]);
  err = uv_cwd(CWD->, size);
  ASSERT(err.code == UV_OK)
}

int main() {
  bootstrap();

  // USER CODE!

  // END OF USER CODE!

  st_delete(&process.cwd);
  free(process);
  return 0;
}

/// IMPL

/// END OF IMPL
