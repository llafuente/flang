// based on https://github.com/victorlcm/HashTable
// interface changed to mach our style

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <stdbool.h>

typedef void* (*hash_malloc_func)(size_t size);
typedef void (*hash_free_func)(void* ptr);
typedef void* (*hash_realloc_func)(void* ptr, size_t size);
typedef void (*hash_each_func)(char* key, void* ptr);

struct hash_entry_s {
  char* key;
  void* value;
  bool bycopy;
  struct hash_entry_s* next;
};

typedef struct hash_entry_s hash_entry_t;

struct hash_s {
  int size;
  hash_entry_t** table;
};

typedef struct hash_s hash_t;

void hash_replace_allocators(hash_malloc_func malloc_func,
                             hash_realloc_func realloc_func,
                             hash_free_func free_func);

// Function to create a hash with the given size
int hash_new(hash_t* ht, int size);

// Function to put the given value with the given size into the specified key
// into the hash table
void hash_set(hash_t* ht, char* key, void* value);

// Function to put the given value with the given size into the specified key
// into the hash table
void hash_set_cp(hash_t* ht, char* key, void* value, int size);

// Function to get the value of the specified key of the hash
void* hash_get(hash_t* ht, char* key);

// Function to get if there is a value in key
bool hash_has(hash_t* ht, char* key);

// Function to get all keys/values
void hash_each(hash_t* ht, hash_each_func each);

// Function to destroy the hash table
void hash_delete(hash_t* ht);

void hash_clear(hash_t* ht);

#define HASH_EACH(hash, var_name, code_block)                                  \
  do {                                                                         \
    hash_entry_t* var_name;                                                    \
                                                                               \
    for (u64 i = 0; i < hash->size; i++) {                                         \
      var_name = hash->table[i];                                               \
      if (var_name != 0) {                                                     \
        do {                                                                   \
          code_block; \
          var_name = var_name->next;                                \
        } while (var_name != 0);                                               \
      }                                                                        \
    }                                                                          \
  } while (false)
