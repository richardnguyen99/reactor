#include "dict.h"

size_t
strhash(const char *str)
{
    size_t hash = FNV_BASIS;

    for (size_t i = 0; str[i]; i++)
    {
        hash ^= str[i];
        hash *= FNV_PRIME;
    }

    return hash;
}

size_t
strihash(const char *str)
{
    size_t hash = FNV_BASIS;

    for (size_t i = 0; str[i]; i++)
    {
        hash ^= tolower(str[i]);
        hash *= FNV_PRIME;
    }

    return hash;
}

int
dict_resize(struct dict *map, size_t capacity)
{
    // clang-format off
    struct dict_entry **buckets = (struct dict_entry **)
                                calloc(capacity, sizeof(struct dict_entry *));
    // clang-format on

    if (buckets == NULL)
        return -1;

    for (size_t i = 0; i < map->capacity; i++)
    {
        struct dict_entry *entry = map->buckets[i];

        while (entry != NULL)
        {
            struct dict_entry *next = entry->next;
            size_t bucket           = map->hash(entry->key) % capacity;

            entry->next     = buckets[bucket];
            buckets[bucket] = entry;
            entry           = next;
        }
    }

    free(map->buckets);
    map->buckets  = buckets;
    map->capacity = capacity;

    return SUCCESS;
}

struct dict *
dict_new(dict_hash_func hash, dict_cmp_func cmp)
{
    struct dict *map = (struct dict *)malloc(sizeof(struct dict));

    if (map == NULL)
        return NULL;

    map->size     = 0;
    map->capacity = DICT_DEFAULT_CAPACITY;
    map->buckets  = (struct dict_entry **)calloc(map->capacity,
                                                 sizeof(struct dict_entry *));
    map->hash     = hash == NULL ? strhash : hash;
    map->cmp      = cmp == NULL ? strcmp : cmp;

    return map;
}

void
dict_delete(struct dict *map)
{
    for (size_t i = 0; i < map->capacity; i++)
    {
        struct dict_entry *entry = map->buckets[i];

        while (entry != NULL)
        {
            struct dict_entry *next = entry->next;
            free(entry->key);
            free(entry->value);
            free(entry);
            entry = next;
        }
    }

    free(map->buckets);
    free(map);
}

int
dict_put(struct dict *map, const char *key, const char *value)
{
    if (map->size >= map->capacity * DICT_LOAD_FACTOR)
    {
        if (dict_resize(map, map->capacity * 2) == -1)
            return -1;
    }

    size_t bucket            = map->hash(key) % map->capacity;
    struct dict_entry *entry = map->buckets[bucket];

    while (entry != NULL)
    {
        if (map->cmp(entry->key, key) == 0)
            return FAILURE;

        entry = entry->next;
    }

    entry = (struct dict_entry *)malloc(sizeof(struct dict_entry));

    if (entry == NULL)
        return ERROR;

    entry->key           = strdup(key);
    entry->value         = strdup(value);
    entry->next          = map->buckets[bucket];
    map->buckets[bucket] = entry;
    map->size++;

    return SUCCESS;
}

char *
dict_get(struct dict *map, const char *key)
{
    size_t bucket            = map->hash(key) % map->capacity;
    struct dict_entry *entry = map->buckets[bucket];

    while (entry != NULL)
    {
        if (map->cmp(entry->key, key) == 0)
            return entry->value;

        entry = entry->next;
    }

    return NULL;
}

int
dict_remove(struct dict *map, const char *key)
{
    size_t bucket            = map->hash(key) % map->capacity;
    struct dict_entry *entry = map->buckets[bucket];
    struct dict_entry *prev  = NULL;

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

        prev  = entry;
        entry = entry->next;
    }

    return FAILURE;
}

struct dict_iterator
dict_begin(struct dict *map)
{
    struct dict_iterator iter;
    memset(&iter, 0, sizeof(dict_iterator_t));

    iter.map = map;

    while (iter.entry == NULL && iter.bucket < iter.map->capacity)
        iter.entry = iter.map->buckets[iter.bucket++];

    return iter;
}

struct dict_iterator
dict_next(struct dict_iterator iter)
{
    if (iter.entry != NULL)
        iter.entry = iter.entry->next;

    while (iter.entry == NULL && iter.bucket < iter.map->capacity)
        iter.entry = iter.map->buckets[iter.bucket++];

    return iter;
}

void
default_iter_func(struct dict_entry *entry, const char *map_name)
{
    map_name = map_name == NULL ? "map" : map_name;

    fprintf(stdout, "%s[\"%s\"] = \"%s\"\n", map_name, entry->key,
            entry->value);
}

void
dict_iterate(struct dict_iterator iter, dict_iter_func func,
             const char *map_name)
{
    if (func == NULL)
        func = default_iter_func;

    while (iter.entry != NULL)
    {
        func(iter.entry, map_name);
        iter = dict_next(iter);
    }
}
