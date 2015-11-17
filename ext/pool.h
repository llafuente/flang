#include <stdlib.h>
#include <stdio.h>
#include "ext/array.h"

typedef struct {
  size_t bytes;
  size_t free;
  void* next;
  char ptr[];
} pool_page_t;

extern array* pool_pages;

extern void pool_init(size_t bytes);
extern void pool_new_page(size_t bytes);
extern void* pool_new(size_t bytes);
extern void pool_destroy();
