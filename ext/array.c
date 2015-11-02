// vector.c

#include <stdio.h>
#include <stdlib.h>
#include "array.h"

void array_new(array *arr) {
  // initialize size and capacity
  arr->size = 0;
  arr->capacity = 100;

  // allocate memory for arr->data
  arr->data = malloc(sizeof(ARRAY_T) * arr->capacity);
}

void array_append(array *arr, ARRAY_T value) {
  // make sure there's room to expand into
  array_double_capacity_if_full(arr);

  // append the value and increment arr->size
  arr->data[arr->size++] = value;
}

ARRAY_T array_get(array *arr, int index) {
  if (index >= arr->size || index < 0) {
    printf("Index %d out of bounds for vector of size %d\n", index, arr->size);
    exit(1);
  }
  return arr->data[index];
}

ARRAY_T array_pop(array *arr) {
  if (arr->size == 0) {
    printf("Cannot pop empty array\n");
    exit(1);
  }

  return arr->data[--arr->size];
}

ARRAY_T array_unshift(array *arr) {
  if (arr->size == 0) {
    printf("Cannot unshift empty array\n");
    exit(1);
  }

  ARRAY_T t = arr->data[0];

  --arr->size;
  if (arr->size) {
    memmove(arr->data, arr->data + 1, sizeof(ARRAY_T) * arr->size);
  }

  return t;
}


void array_set(array *arr, int index, ARRAY_T value) {
  // zero fill the vector up to the desired index
  while (index >= arr->size) {
    array_append(arr, 0);
  }

  // set the value at the desired index
  arr->data[index] = value;
}

void array_double_capacity_if_full(array *arr) {
  if (arr->size >= arr->capacity) {
    arr->capacity = arr->size + 50;
    arr->data = realloc(arr->data, sizeof(ARRAY_T) * arr->capacity);
  }
}

void array_delete(array *arr) {
  free(arr->data);
}
