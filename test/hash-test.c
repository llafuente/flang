/*
* Copyright 2015 Luis Lafuente <llafuente@noboxout.com>
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
* 1. Redistributions of source code must retain the above copyright
* notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in the
* documentation and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
* THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "flang.h"
#include "tasks.h"
#include "test.h"

size_t __hash_counter_it = 0;
void __hash_counter(char* key, void* ptr) { ++__hash_counter_it; }

void hash_dump(hash_t* ht) { printf("ht->size %d\n", ht->size); }
// TODO review if ";" is required
TASK_IMPL(hash) {
  size_t a = 1, b = 2, c = 3;
  hash_t* ht = malloc(sizeof(hash_t));

  ASSERT(hash_new(ht, 101) == 0, "new hash failed");
  hash_dump(ht);
  printf("h->size out = %d\n", (ht)->size);

  hash_set_cp(ht, "test1", &a, sizeof(size_t));
  size_t* x = hash_get(ht, "test1");
  ASSERT(*x == a, "test1 = 1");

  hash_set(ht, "test2", &b);
  x = hash_get(ht, "test2");
  ASSERT(*x == b, "test2 = 2");

  hash_set(ht, "test3", &c);
  x = hash_get(ht, "test3");
  ASSERT(*x == c, "test3 = 2");

  size_t* number = malloc(sizeof(size_t));
  *number = 101;

  hash_set_cp(ht, "test4", number, sizeof(size_t));
  x = hash_get(ht, "test4");
  ASSERT(*x == *number, "test4 = 2");

  hash_set(ht, "test5", number);
  x = hash_get(ht, "test5");
  ASSERT(*x == *number, "test5 = 2");

  free(number);
  x = hash_get(ht, "test4"); // works
  ASSERT(*x == 101, "test4 = 2");

  // this test is commented because cannot be tested
  // x = hash_get(ht, "test5"); // usage after free
  // ASSERT(*x == *number, "test4 = 2");

  x = hash_get(ht, "testx");
  ASSERT(x == 0, "testx is null");

  hash_each(ht, __hash_counter);

  ASSERT(__hash_counter_it == 5, "hash size!");

  hash_delete(ht);
  free(ht);

  return 0;
}
