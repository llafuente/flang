// vector.c

#include <stdio.h>
#include <stdlib.h>
#include "flang/flang.h"

array_malloc_func __array_replaced_malloc = 0;
array_free_func __array_replaced_free = 0;
array_realloc_func __array_replaced_realloc = 0;

void* __array_malloc(size_t size) {
  if (__array_replaced_malloc)
    return (*__array_replaced_malloc)(size);
  return malloc(size);
}

void __array_free(void* ptr) {
  if (__array_replaced_free)
    (*__array_replaced_free)(ptr);
  else
    free(ptr);
}

void* __array_realloc(void* ptr, size_t size) {
  if (__array_replaced_realloc)
    return (*__array_replaced_realloc)(ptr, size);

  return realloc(ptr, size);
}

void array_replace_allocators(array_malloc_func malloc_func,
                              array_realloc_func realloc_func,
                              array_free_func free_func) {
  if (malloc_func) {
    __array_replaced_malloc = malloc_func;
  }

  if (realloc_func) {
    __array_replaced_realloc = realloc_func;
  }

  if (free_func) {
    __array_replaced_free = free_func;
  }
}

void array_new(array* arr) {
  // initialize size and capacity
  arr->length = 0;
  arr->capacity = 10;

  // allocate memory for arr->values
  arr->values = __array_malloc(sizeof(ARRAY_T) * arr->capacity);
}

void array_newcap(array* arr, size_t cap) {
  // initialize size and capacity
  arr->length = 0;
  arr->capacity = cap;

  // allocate memory for arr->values
  arr->values = __array_malloc(sizeof(ARRAY_T) * cap);
}

void array_push(array* arr, ARRAY_T value) {
  // make sure there's room to expand into
  array_double_capacity_if_full(arr);

  // append the value and increment arr->length
  arr->values[arr->length++] = value;
}

void array_push_unique(array* arr, ARRAY_T value) {
  for (u64 i = 0; i < arr->length; ++i) {
    if (arr->values[i] == value) {
      return;
    }
  }

  array_push(arr, value);
}

void array_concat(array* arr, array* arr2) {
  size_t i;
  for (i = 0; i < arr2->length; ++i) {
    array_push(arr, arr2->values[i]);
  }
}

ARRAY_T array_get(array* arr, int index) {
  if (index >= arr->length || index < 0) {
    fprintf(stderr, "Index %d out of bounds for vector of size %lu\n", index,
            arr->length);
    exit(1);
  }
  return arr->values[index];
}

ARRAY_T array_pop(array* arr) {
  if (arr->length == 0) {
    fprintf(stderr, "Cannot pop empty array\n");
    exit(1);
  }

  return arr->values[--arr->length];
}

ARRAY_T array_unshift(array* arr) {
  if (arr->length == 0) {
    fprintf(stderr, "Cannot unshift empty array\n");
    exit(1);
  }

  ARRAY_T t = arr->values[0];

  --arr->length;
  if (arr->length) {
    memmove(arr->values, arr->values + 1, sizeof(ARRAY_T) * arr->length);
  }

  return t;
}

void array_insert(array* arr, u64 index, ARRAY_T value) {
  array_double_capacity_if_full(arr);

  memmove(arr->values + index + 1, arr->values + index,
          sizeof(ARRAY_T) * (arr->length - index));

  arr->values[index] = value;
  ++arr->length;
}

void array_set(array* arr, int index, ARRAY_T value) {
  // zero fill the vector up to the desired index
  while (index >= arr->length) {
    array_push(arr, 0);
  }

  // set the value at the desired index
  arr->values[index] = value;
}

void array_double_capacity_if_full(array* arr) {
  if (!arr->capacity) {
    arr->capacity = 8;
    arr->values = __array_malloc(sizeof(ARRAY_T) * arr->capacity);
  } else if (arr->length >= arr->capacity) {
    arr->capacity = arr->length + 50;
    ARRAY_T p = __array_realloc(arr->values, sizeof(ARRAY_T) * arr->capacity);
    // TODO this shouldn't be necessary, pool_realloc is buggy!
    memcpy(p, arr->values, sizeof(ARRAY_T) * arr->length);
    __array_free(arr->values);
    arr->values = p;
  }
}

void array_delete(array* arr) { __array_free(arr->values); }
