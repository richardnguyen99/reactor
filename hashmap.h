#ifndef _JAWS_HASHMAP_H
#define _JAWS_HASHMAP_H 1

#include "defs.h"

/**
 * Header for hash map for string keys and string values
 */

#define HASHMAP_DEFAULT_CAPACITY 16
#define HASHMAP_DEFAULT_LOAD_FACTOR 0.75
#define FNV_PRIME 0x01000193
#define FNV_BASIS 0x811C9DC5

size_t strhash(const char *str);
size_t strihash(const char *str);

struct hashmap;
struct hashmap_iterator;
struct hashmap_entry;

typedef struct hashmap hashmap_t;
typedef struct hashmap_iterator hashmap_iterator_t;
typedef struct hashmap_entry hashmap_entry_t;

typedef size_t (*hashmap_hash_func)(const char *);
typedef int (*hashmap_cmp_func)(const char *, const char *);

/**
 * @brief Create a new hash map object
 *
 * @param hash Hash function for the hash map
 * @param cmp Compare function for the hash map
 * @return hashmap_t* Pointer to the hash map object
 */
hashmap_t *hashmap_new(hashmap_hash_func hash, hashmap_cmp_func cmp);

/**
 * @brief  Put a key-value pair into the hash map
 *
 * @param map Pointer to the hash map object
 * @param key  Key to be inserted
 * @param value Value to be inserted
 * @return int 1 if success, 0 if there exists a key-value pair with the same key, -1 if error.
 */
int hashmap_put(hashmap_t *map, const char *key, const char *value);

#endif // _JAWS_HASHMAP_H
