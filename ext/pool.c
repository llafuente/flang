#include "flang/flang.h"
#include <math.h>

array* pool_pages = 0;
size_t pool_page_size = 0;

void pool_init(size_t bytes) {
  pool_pages = calloc(sizeof(array), 1);
  // array_new(pool_pages);
  pool_pages->length = 0;
  pool_pages->capacity = 100;
  pool_pages->values = calloc(sizeof(ARRAY_T), 100);

  pool_new_page(bytes);
  pool_page_size = bytes;
}

void pool_new_page(size_t bytes) {
  if (pool_pages->length == pool_pages->capacity) {
    pool_pages->capacity = pool_pages->length + 50;
    pool_pages->values =
        realloc(pool_pages->values, sizeof(ARRAY_T) * pool_pages->capacity);
  }

  pool_page_t* page = (pool_page_t*)calloc(sizeof(pool_page_t) + bytes, 1);
  page->next = page->ptr;
  page->free = bytes;
  array_push(pool_pages, (void*)page);
}

void* pool_new(size_t bytes) {
  // everything in the pool is 8 bytes alligned
  float bt = bytes;
  bt /= 8;
  bt = ceil(bt);
  bt *= 8;
  bytes = bt;

  size_t i = 0;
  for (; i < pool_pages->length; ++i) {
    pool_page_t* p = (pool_page_t*)pool_pages->values[i];
    if (p->free >= bytes) {
      void* ret = p->next;

      p->next += bytes;
      p->free -= bytes;

      return ret;
    }
  }

  pool_new_page(bytes > pool_page_size ? bytes : pool_page_size);
  return pool_new(bytes);
}

// TODO this is a HUGE bug mess!! because we don't know the old size!
void* pool_realloc(void* ptr, size_t bytes) { return pool_new(bytes); }
void pool_free(void* ptr) {}

void pool_destroy() {
  size_t i = 0;
  for (; i < pool_pages->length; ++i) {
    free(pool_pages->values[i]);
  }

  free(pool_pages->values);
  free(pool_pages);
  pool_pages = 0;
}
