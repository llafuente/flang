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

# ifdef __GNUC__
#  define force_inline      __attribute__ ((always_inline)) inline
# elif defined(_MSC_VER)
#  define force_inline      __forceinline
# else
#  define force_inline
# endif

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

//
// types are incomplete atm. need review on
// what to expose, and how inside the language...
//

typedef enum {
  FL_VOID = 1,
  FL_NUMBER = 2,
  FL_POINTER = 3, // wrapper
  FL_VECTOR = 4,  // wrapper
  FL_FUNCTION = 5,
  FL_STRUCT = 6,
  // FL_ENUM = 7, // TODO this is in fact an "int"

  FL_INFER = 10,

  FL_TEMPLATE = 20,
} types_t;

typedef struct {
  string* id;
  types_t of;
  union {
    struct {
      unsigned char bits;
      bool fp;
      bool sign;
    } number;
  };
} type_t;

/// DECL

/// END OF DECL

void bootstrap() {
  process = malloc(sizeof(process_t));
  process->cwd = st_new(MAX_PATH, st_enc_utf8);
  size_t size = 1027;
  int err = uv_cwd(process->cwd->value, &size);
  assert(err == 0);
}

void run();

int main() {
  bootstrap();

  run();

  st_delete(&process->cwd);
  free(process);
  return 0;
}

/// IMPL

/// END OF IMPL
