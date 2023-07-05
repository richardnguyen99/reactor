#include "hashmap.h"

size_t strhash(const char *str)
{
    size_t hash = FNV_BASIS;

    for (size_t i = 0; str[i]; i++)
    {
        hash ^= str[i];
        hash *= FNV_PRIME;
    }

    return hash;
}

size_t strihash(const char *str)
{
    size_t hash = FNV_BASIS;

    for (size_t i = 0; str[i]; i++)
    {
        hash ^= tolower(str[i]);
        hash *= FNV_PRIME;
    }

    return hash;
}

struct hashmap_entry
{
    char *key;
    char *value;
    struct hashmap_entry *next;
};

struct hashmap_iterator
{
    struct hashmap *map;
    size_t bucket;
    struct hashmap_entry *entry;
};

struct hashmap
{
    size_t size;
    size_t capacity;
    struct hashmap_entry **buckets;

    hashmap_hash_func hash;
    hashmap_cmp_func cmp;
};

int hashmap_resize(struct hashmap *map, size_t capacity)
{
    struct hashmap_entry **buckets = ((struct hashmap_entry **)
                                          calloc(capacity,
                                                 sizeof(struct hashmap_entry *)));

    if (buckets == NULL)
        return -1;

    for (size_t i = 0; i < map->capacity; i++)
    {
        struct hashmap_entry *entry = map->buckets[i];

        while (entry != NULL)
        {
            struct hashmap_entry *next = entry->next;
            size_t bucket = map->hash(entry->key) % capacity;

            entry->next = buckets[bucket];
            buckets[bucket] = entry;
            entry = next;
        }
    }

    free(map->buckets);
    map->buckets = buckets;
    map->capacity = capacity;

    return SUCCESS;
}

struct hashmap *hashmap_new(hashmap_hash_func hash, hashmap_cmp_func cmp)
{
    struct hashmap *map = (struct hashmap *)malloc(sizeof(struct hashmap));

    if (map == NULL)
        return NULL;

    map->size = 0;
    map->capacity = HASHMAP_DEFAULT_CAPACITY;
    map->buckets = (struct hashmap_entry **)calloc(map->capacity, sizeof(struct hashmap_entry *));
    map->hash = hash == NULL ? strhash : hash;
    map->cmp = cmp == NULL ? strcmp : cmp;

    return map;
}

int hashmap_put(struct hashmap *map, const char *key, const char *value)
{
    if (map->size >= map->capacity * HASHMAP_DEFAULT_LOAD_FACTOR)
    {
        if (hashmap_resize(map, map->capacity * 2) == -1)
            return -1;
    }

    size_t bucket = map->hash(key) % map->capacity;
    struct hashmap_entry *entry = map->buckets[bucket];

    while (entry != NULL)
    {
        if (map->cmp(entry->key, key) == 0)
            return FAILURE;

        entry = entry->next;
    }

    entry = (struct hashmap_entry *)malloc(sizeof(struct hashmap_entry));

    if (entry == NULL)
        return ERROR;

    entry->key = strdup(key);
    entry->value = strdup(value);
    entry->next = map->buckets[bucket];
    map->buckets[bucket] = entry;
    map->size++;

    return SUCCESS;
}
