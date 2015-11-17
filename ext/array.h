// based on: http://www.happybearsoftware.com/implementing-a-dynamic-array.html
// vector.h

#include <string.h>

#ifndef ARRAY_T
// Define a vector type
typedef struct {
  int size;
  int capacity;
  void **data;     // array of pointers
} array;
#endif

#define VECTOR_INITIAL_CAPACITY 100
#define ARRAY_T void*


void array_new(array *arr);

void array_append(array *arr, ARRAY_T value);

ARRAY_T array_get(array *arr, int index);

ARRAY_T array_pop(array *arr);

ARRAY_T array_unshift(array *arr);

void array_set(array *arr, int index, ARRAY_T value);

void array_double_capacity_if_full(array *arr);

void array_delete(array *arr);
