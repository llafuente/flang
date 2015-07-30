// from https://gist.github.com/tonious/1377667
// until time to get a better one

struct entry_s {
  char *key;
  size_t value;
  struct entry_s *next;
};

typedef struct entry_s entry_t;

struct hashtable_s {
  int size;
  struct entry_s **table;
};

typedef struct hashtable_s hashtable_t;

extern hashtable_t *ht_create( int size );
extern int ht_hash( hashtable_t *hashtable, char *key );
extern entry_t *ht_newpair( char *key, size_t value );
extern void ht_set( hashtable_t *hashtable, char *key, size_t value );
extern size_t ht_get( hashtable_t *hashtable, char *key );
extern void ht_free(hashtable_t *hashtable);
