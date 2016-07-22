// based on: http://www.happybearsoftware.com/implementing-a-dynamic-array.html
// vector.h

#include <string.h>

#ifndef ARRAY_T
// Define a vector type
typedef struct {
  u64 length;
  u64 capacity;
  (void*)* values; // array of pointers void*
} array;

typedef void* (*array_malloc_func)(size_t size);
typedef void (*array_free_func)(void* ptr);
typedef void* (*array_realloc_func)(void* ptr, size_t size);
typedef void (*array_each_func)(char* key, void* ptr);
#endif

#define VECTOR_INITIAL_CAPACITY 100
#define ARRAY_T void *

void array_replace_allocators(array_malloc_func malloc_func,
                              array_realloc_func realloc_func,
                              array_free_func free_func);

void array_new(array* arr);
void array_newcap(array* arr, size_t cap);

void array_push(array* arr, ARRAY_T value);

void array_concat(array* arr, array* arr2);

ARRAY_T array_get(array* arr, int index);

ARRAY_T array_pop(array* arr);

ARRAY_T array_unshift(array* arr);

void array_set(array* arr, int index, ARRAY_T value);

void array_double_capacity_if_full(array* arr);

void array_delete(array* arr);
