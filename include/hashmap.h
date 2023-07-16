#ifndef _REACTOR_HASHMAP_H
#define _REACTOR_HASHMAP_H 1

#include "defs.h"

/**
 * Header for hash map for string keys and string values
 */

#define HASHMAP_DEFAULT_CAPACITY 16
#define HASHMAP_DEFAULT_LOAD_FACTOR 0.75
#define FNV_PRIME 0x01000193
#define FNV_BASIS 0x811C9DC5

#define DEFAULT_ITER_FUNC NULL
#define DEFAULT_MAP_NAME "map"

size_t strhash(const char *str);
size_t strihash(const char *str);

typedef size_t (*hashmap_hash_func)(const char *);
typedef int (*hashmap_cmp_func)(const char *, const char *);
struct hashmap_entry
{
    char *key;
    char *value;
    struct hashmap_entry *next;
};

struct hashmap
{
    size_t size;
    size_t capacity;
    struct hashmap_entry **buckets;

    hashmap_hash_func hash;
    hashmap_cmp_func cmp;
};

struct hashmap_iterator
{
    struct hashmap *map;
    size_t bucket;
    struct hashmap_entry *entry;
};

typedef struct hashmap hashmap_t;
typedef struct hashmap_entry hashmap_entry_t;
typedef struct hashmap_iterator hashmap_iterator_t;

typedef void (*hashmap_iter_func)(struct hashmap_entry *, const char *);

/**
 * @brief Create a new hash map object
 *
 * @param hash Hash function for the hash map
 * @param cmp Compare function for the hash map
 * @return hashmap_t* Pointer to the hash map object
 */
hashmap_t *hashmap_new(hashmap_hash_func hash, hashmap_cmp_func cmp);

/**
 * @brief  Delete a hash map object
 *
 * @param map Pointer to the hash map object
 */
void hashmap_delete(struct hashmap *map);

/**
 * @brief  Put a key-value pair into the hash map
 *
 * @param map Pointer to the hash map object
 * @param key  Key to be inserted
 * @param value Value to be inserted
 * @return int 0 if success, 1 if there exists a key-value pair with the same key, -1 if error.
 */
int hashmap_put(struct hashmap *map, const char *key, const char *value);

/**
 * @brief  Get the value of a key
 *
 * @param map Pointer to the hash map object
 * @param key Key to be searched
 * @return char* Value of the key, NULL if not found
 */
char *hashmap_get(struct hashmap *map, const char *key);

/**
 * @brief  Remove a key-value pair from the hash map
 *
 * @param map Pointer to the hash map object
 * @param key Key to be removed
 * @return int 1 if success, 0 if not found, -1 if error
 */
int hashmap_remove(struct hashmap *map, const char *key);

hashmap_iterator_t hashmap_begin(struct hashmap *map);
hashmap_iterator_t hashmap_next(struct hashmap_iterator iter);
void hashmap_iterate(struct hashmap_iterator iter, hashmap_iter_func iter_func, const char *map_name);

#endif // _REACTOR_HASHMAP_H
