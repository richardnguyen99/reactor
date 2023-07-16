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

void hashmap_delete(struct hashmap *map)
{
    for (size_t i = 0; i < map->capacity; i++)
    {
        struct hashmap_entry *entry = map->buckets[i];

        while (entry != NULL)
        {
            struct hashmap_entry *next = entry->next;
            free(entry->key);
            free(entry->value);
            free(entry);
            entry = next;
        }
    }

    free(map->buckets);
    free(map);
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

char *hashmap_get(struct hashmap *map, const char *key)
{
    size_t bucket = map->hash(key) % map->capacity;
    struct hashmap_entry *entry = map->buckets[bucket];

    while (entry != NULL)
    {
        if (map->cmp(entry->key, key) == 0)
            return entry->value;

        entry = entry->next;
    }

    return NULL;
}

int hashmap_remove(struct hashmap *map, const char *key)
{
    size_t bucket = map->hash(key) % map->capacity;
    struct hashmap_entry *entry = map->buckets[bucket];
    struct hashmap_entry *prev = NULL;

    while (entry != NULL)
    {
        if (map->cmp(entry->key, key) == 0)
        {
            if (prev == NULL)
                map->buckets[bucket] = entry->next;
            else
                prev->next = entry->next;

            free(entry->key);
            free(entry->value);
            free(entry);
            map->size--;

            return SUCCESS;
        }

        prev = entry;
        entry = entry->next;
    }

    return FAILURE;
}

struct hashmap_iterator hashmap_begin(hashmap_t *map)
{
    struct hashmap_iterator iter;
    memset(&iter, 0, sizeof(hashmap_iterator_t));

    iter.map = map;

    while (iter.entry == NULL && iter.bucket < iter.map->capacity)
        iter.entry = iter.map->buckets[iter.bucket++];

    return iter;
}

struct hashmap_iterator hashmap_next(struct hashmap_iterator iter)
{
    if (iter.entry != NULL)
        iter.entry = iter.entry->next;

    while (iter.entry == NULL && iter.bucket < iter.map->capacity)
        iter.entry = iter.map->buckets[iter.bucket++];

    return iter;
}

void default_iter_func(struct hashmap_entry *entry, const char *map_name)
{
    map_name = map_name == NULL ? "map" : map_name;

    fprintf(stdout, "%s[\"%s\"] = \"%s\"\n", map_name, entry->key, entry->value);
}

void hashmap_iterate(struct hashmap_iterator iter, hashmap_iter_func func, const char *map_name)
{
    if (func == NULL)
        func = default_iter_func;

    while (iter.entry != NULL)
    {
        func(iter.entry, map_name);
        iter = hashmap_next(iter);
    }
}
