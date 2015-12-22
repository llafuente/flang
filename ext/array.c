// vector.c

#include <stdio.h>
#include <stdlib.h>
#include "array.h"

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
  arr->size = 0;
  arr->capacity = 10;

  // allocate memory for arr->data
  arr->data = __array_malloc(sizeof(ARRAY_T) * arr->capacity);
}

void array_append(array* arr, ARRAY_T value) {
  // make sure there's room to expand into
  array_double_capacity_if_full(arr);

  // append the value and increment arr->size
  arr->data[arr->size++] = value;
}

ARRAY_T array_get(array* arr, int index) {
  if (index >= arr->size || index < 0) {
    fprintf(stderr, "Index %d out of bounds for vector of size %d\n", index,
            arr->size);
    exit(1);
  }
  return arr->data[index];
}

ARRAY_T array_pop(array* arr) {
  if (arr->size == 0) {
    fprintf(stderr, "Cannot pop empty array\n");
    exit(1);
  }

  return arr->data[--arr->size];
}

ARRAY_T array_unshift(array* arr) {
  if (arr->size == 0) {
    fprintf(stderr, "Cannot unshift empty array\n");
    exit(1);
  }

  ARRAY_T t = arr->data[0];

  --arr->size;
  if (arr->size) {
    memmove(arr->data, arr->data + 1, sizeof(ARRAY_T) * arr->size);
  }

  return t;
}

void array_set(array* arr, int index, ARRAY_T value) {
  // zero fill the vector up to the desired index
  while (index >= arr->size) {
    array_append(arr, 0);
  }

  // set the value at the desired index
  arr->data[index] = value;
}

void array_double_capacity_if_full(array* arr) {
  if (arr->size >= arr->capacity) {
    arr->capacity = arr->size + 50;
    arr->data = __array_realloc(arr->data, sizeof(ARRAY_T) * arr->capacity);
  }
}

void array_delete(array* arr) { __array_free(arr->data); }
