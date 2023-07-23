/**
 * @file dict.h
 * @author Richard Nguyen (richard@richardhnguyen.com)
 * @brief Dictionary data structure header file
 * @version 0.1
 * @date 2023-07-19
 *
 * @copyright Copyright (c) 2023
 */

#ifndef _REACTOR_DICT_H_
#define _REACTOR_DICT_H_ 1

#include "defs.h"

/**
 * Header for hash map for string keys and string values
 */

#define DICT_DEFAULT_CAPACITY 16
#define DICT_LOAD_FACTOR      0.75
#define FNV_PRIME             0x01000193
#define FNV_BASIS             0x811C9DC5

#define DEFAULT_ITER_FUNC NULL
#define DEFAULT_MAP_NAME  "map"

// clang-format off
size_t strhash(const char *str);
size_t strihash(const char *str);
// clang-format on

typedef size_t (*dict_hash_func)(const char *);
typedef int (*dict_cmp_func)(const char *, const char *);

struct dict_entry
{
    char *key;
    char *value;
    struct dict_entry *next;
};

struct dict
{
    size_t size;
    size_t capacity;
    struct dict_entry **buckets;

    dict_hash_func hash;
    dict_cmp_func cmp;
};

struct dict_iterator
{
    struct dict *map;
    size_t bucket;
    struct dict_entry *entry;
};

typedef struct dict_iterator dict_iterator_t;

typedef void (*dict_iter_func)(struct dict_entry *, const char *);

/**
 * @brief Create a new hash map object
 *
 * @param hash Hash function for the hash map
 * @param cmp Compare function for the hash map
 * @return dict_t* Pointer to the hash map object
 */
struct dict *
dict_new(dict_hash_func hash, dict_cmp_func cmp);

/**
 * @brief  Delete a hash map object
 *
 * @param map Pointer to the hash map object
 */
void
dict_delete(struct dict *map);

/**
 * @brief  Put a key-value pair into the hash map
 *
 * @param map Pointer to the hash map object
 * @param key  Key to be inserted
 * @param value Value to be inserted
 * @return int 0 if success, 1 if there exists a key-value pair with the same
 * key, -1 if error.
 */
int
dict_put(struct dict *map, const char *key, const char *value);

/**
 * @brief  Get the value of a key
 *
 * @param map Pointer to the hash map object
 * @param key Key to be searched
 * @return char* Value of the key, NULL if not found
 */
char *
dict_get(struct dict *map, const char *key);

/**
 * @brief  Remove a key-value pair from the hash map
 *
 * @param map Pointer to the hash map object
 * @param key Key to be removed
 * @return int 1 if success, 0 if not found, -1 if error
 */
int
dict_remove(struct dict *map, const char *key);

dict_iterator_t
dict_begin(struct dict *map);

dict_iterator_t
dict_next(struct dict_iterator iter);

void
dict_iterate(struct dict_iterator iter, dict_iter_func iter_func,
             const char *map_name);

#endif // _REACTOR_DICT_H_
