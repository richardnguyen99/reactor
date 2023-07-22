#include "threads.h"

struct thread_pool *
pool_new(size_t no_threads, size_t buf_size, thread_func func)
{
    struct thread_pool *pool =
        (struct thread_pool *)malloc(sizeof(struct thread_pool));

    if (pool == NULL)
        return NULL;

    pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * no_threads);

    if (pool->threads == NULL)
        DIE("(pool_new) malloc");

    if (pthread_mutex_init(&(pool->lock), NULL) == -1)
        DIE("(pthread_init)");

    if (sem_init(&(pool->full), 0, 0) == -1)
        DIE("(pool_new) sem_init");

    if (sem_init(&(pool->empty), 0, buf_size) == -1)
        DIE("(pool_new) sem_init");

    pool->buffer = rbuffer_new(buf_size);
    if (pool->buffer == NULL)
        DIE("(pool_new) rbuffer_new");

    pool->size = no_threads;
    pool->func = func;

    return pool;
}

void
pool_free(struct thread_pool *pool)
{
    if (pool == NULL)
        return;

    if (pool->buffer != NULL)
        rbuffer_free(pool->buffer);

    if (pool->threads != NULL)
        free(pool->threads);

    if (sem_destroy(&(pool->full)) == -1)
        DIE("(pool_free) sem_destroy");

    if (sem_destroy(&(pool->empty)) == -1)
        DIE("(pool_free) sem_destroy");

    free(pool);
}
