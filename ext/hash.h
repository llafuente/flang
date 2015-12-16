// based on https://github.com/victorlcm/HashTable
// interface changed to mach our style

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <stdbool.h>

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

// Function to destroy the hash table
void hash_delete(hash_t* ht);
