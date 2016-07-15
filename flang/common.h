#include <inttypes.h>
#include <stdbool.h> // import bool

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

#define libexport extern

#include "ext/hash.h"  // import hash_t
#include "ext/array.h" // import array_t
#include "ext/pool.h"  // import pool_t
#include "stringc.h"   // import string

// defined @flang-typesystem
enum ts_types;
typedef enum ts_types ts_types_t;

struct ts_type;
typedef struct ts_type ty_t;

// defined @flang-ast
enum ast_action;
typedef enum ast_action ast_action_t;

enum ast_cast_operations;
typedef enum ast_cast_operations ast_cast_operations_t;

enum ast_scope;
typedef enum ast_scope ast_scope_t;

enum ast_types;
typedef enum ast_types ast_types_t;

struct ast;
typedef struct ast ast_t;

struct ast;
typedef enum ast_trav_mode ast_trav_mode_t;

// callback type for [ast_traverse](#ast_traverse) & [ast_reverse](#ast_reverse)
typedef ast_action_t (*ast_cb_t)(ast_trav_mode_t mode, ast_t* node,
                                 ast_t* parent, u64 level, void* userdata_in,
                                 void* userdata_out);
