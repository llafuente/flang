#include "hash.h"

hash_malloc_func __hash_replaced_malloc = 0;
hash_free_func __hash_replaced_free = 0;
hash_realloc_func __hash_replaced_realloc = 0;

void* __hash_malloc(size_t size) {
  if (__hash_replaced_malloc)
    return (*__hash_replaced_malloc)(size);
  return malloc(size);
}

void __hash_free(void* ptr) {
  if (__hash_replaced_free)
    (*__hash_replaced_free)(ptr);
  else
    free(ptr);
}

void* __hash_realloc(void* ptr, size_t size) {
  if (__hash_replaced_realloc)
    return (*__hash_replaced_realloc)(ptr, size);

  return realloc(ptr, size);
}

void hash_replace_allocators(hash_malloc_func malloc_func,
                             hash_realloc_func realloc_func,
                             hash_free_func free_func) {
  if (malloc_func) {
    __hash_replaced_malloc = malloc_func;
  }

  if (realloc_func) {
    __hash_replaced_realloc = realloc_func;
  }

  if (free_func) {
    __hash_replaced_free = free_func;
  }
}

// Function to destroy the hash table
void hash_delete(hash_t* ht) {
  int i;
  hash_entry_t* entry;
  hash_entry_t* next;

  // Free each not empty position of the hash table
  for (i = 0; i < ht->size; i++) {
    entry = ht->table[i];
    if (entry != 0) {
      do {
        if (entry->bycopy) {
          __hash_free(entry->key);
          __hash_free(entry->value);
        }
        next = entry->next;
        __hash_free(entry);
        entry = next;
      } while (entry != 0);
    }
  }
  __hash_free(ht->table);
  ht->table = 0; // no dangling
}

// Function to create a hash with the given size
int hash_new(hash_t* ht, int size) {

  if (size < 1) {
    return 1;
  }

  // Allocation of the pointer of the linked lists of each hashCode
  if ((ht->table = __hash_malloc(sizeof(struct hash_entry_s*) * size)) == 0) {
    return 1;
  }

  // Initializing every pointer as 0
  int i;
  for (i = 0; i < size; i++) {
    ht->table[i] = 0;
  }

  // Setting the size
  ht->size = size;

  return 0;
}

// This function is used to hash a string to be mapped into the hash table
static int hash_function(hash_t* ht, char* key) {

  unsigned long int hash_val = 5381;

  unsigned int index = 0;

  while (hash_val < ULONG_MAX && index < strlen(key)) {
    hash_val = hash_val << 8;
    hash_val += key[index];
    index++;
  }

  return hash_val % ht->size;
}

// Funtion to create an entry with the given key value and size of the value
hash_entry_t* hash_new_entry(hash_t* ht, char* key, void* value) {

  hash_entry_t* new_entry;

  if ((new_entry = __hash_malloc(sizeof(struct hash_entry_s))) == 0) {
    return 0;
  }
  // do not clone value or key
  new_entry->key = key;
  new_entry->value = value;

  new_entry->next = 0;

  return new_entry;
}

hash_entry_t* hash_new_entry_cp(hash_t* ht, char* key, void* value, int size) {

  hash_entry_t* new_entry;

  if ((new_entry = __hash_malloc(sizeof(struct hash_entry_s))) == 0) {
    return 0;
  }

  if ((new_entry->key = strdup(key)) == 0) {
    return 0;
  }

  if ((new_entry->value = __hash_malloc(size)) == 0) {
    return 0;
  }

  memcpy(new_entry->value, value, size);

  new_entry->next = 0;
  new_entry->bycopy = true;

  return new_entry;
}

// function to put the given value with the given size into the specified key
// into the hash table
void hash_set(hash_t* ht, char* key, void* value) {

  int hash_code = 0;

  // Get the hash code value from the hash function
  hash_code = hash_function(ht, key);

  hash_entry_t* current_item = 0;

  // set the current item as the one found by the hash function
  current_item = ht->table[hash_code];

  hash_entry_t* previous_item = 0;

  // Try to find the specified key into the hash table
  while (current_item != 0 && current_item->key != 0 &&
         strcmp(key, current_item->key) > 0) {
    previous_item = current_item;
    current_item = current_item->next;
  }

  // Case there is already an entry in that key, replace that entry value
  if (current_item != 0 && current_item->key != 0 &&
      strcmp(key, current_item->key) == 0) {
    if (current_item->bycopy) {
      __hash_free(current_item->value);
    }

    current_item->bycopy = false;
    current_item->value = value;
  }
  // Case there is NOT an entry with that key, create a new entry
  else {
    hash_entry_t* new_entry = 0;

    new_entry = hash_new_entry(ht, key, value);

    // Start of the linked list in this hash code
    if (current_item == ht->table[hash_code]) {
      new_entry->next = current_item;
      ht->table[hash_code] = new_entry;

      // End of the linked list in this hash code
    } else if (current_item == 0) {
      previous_item->next = new_entry;

      // Middle of the list
    } else {
      new_entry->next = current_item;
      previous_item->next = new_entry;
    }
  }
}

// function to put the given value with the given size into the specified key
// into the hash table
void hash_set_cp(hash_t* ht, char* key, void* value, int size) {

  int hash_code = 0;

  // Get the hash code value from the hash function
  hash_code = hash_function(ht, key);

  hash_entry_t* current_item = 0;

  // set the current item as the one found by the hash function
  current_item = ht->table[hash_code];

  hash_entry_t* previous_item = 0;

  // Try to find the specified key into the hash table
  while (current_item != 0 && current_item->key != 0 &&
         strcmp(key, current_item->key) > 0) {
    previous_item = current_item;
    current_item = current_item->next;
  }

  // Case there is already an entry in that key, replace that entry value
  if (current_item != 0 && current_item->key != 0 &&
      strcmp(key, current_item->key) == 0) {

    __hash_free(current_item->value);

    // TODO use library for error handling later
    if ((current_item->value = __hash_malloc(size)) == 0) {
      exit(1);
    }

    memcpy(current_item->value, value, size);

  }
  // Case there is NOT an entry with that key, create a new entry
  else {

    hash_entry_t* new_entry = 0;

    new_entry = hash_new_entry_cp(ht, key, value, size);

    // Start of the linked list in this hash code
    if (current_item == ht->table[hash_code]) {
      new_entry->next = current_item;
      ht->table[hash_code] = new_entry;

      // End of the linked list in this hash code
    } else if (current_item == 0) {

      previous_item->next = new_entry;

      // Middle of the list
    } else {
      new_entry->next = current_item;
      previous_item->next = new_entry;
    }
  }
}

// function to get the value of the specified key of the hash
void* hash_get(hash_t* ht, char* key) {
  int hash_code = hash_function(ht, key);

  hash_entry_t* entry = ht->table[hash_code];

  // Step through the hash code, looking for the value
  while (entry != 0 && entry->key != 0 && strcmp(key, entry->key) > 0) {
    entry = entry->next;
  }

  // If nothing was found return null
  if (entry == 0 || entry->key == 0 || strcmp(key, entry->key) != 0) {
    return 0;
  }

  return entry->value;
}

bool hash_has(hash_t* ht, char* key) { return hash_get(ht, key) != 0; }

void hash_each(hash_t* ht, hash_each_func each) {
  int i;
  hash_entry_t* entry;

  // Free each not empty position of the hash table
  for (i = 0; i < ht->size; i++) {
    entry = ht->table[i];
    if (entry != 0) {
      do {
        each(entry->key, entry->value);
        entry = entry->next;
      } while (entry != 0);
    }
  }
}

void hash_clear(hash_t* ht) {
  int i;
  hash_entry_t* entry;

  // Free each not empty position of the hash table
  for (i = 0; i < ht->size; i++) {
    ht->table[i] = 0;
  }
}
