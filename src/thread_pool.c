#include "thread_pool.h"

struct thread_pool *tp_create(size_t queue_cap, size_t pool_size )
{
    struct thread_pool *pool = (struct thread_pool *)malloc(sizeof(struct thread_pool));

    if (pool == NULL)
        return NULL;

    pthread_t *threads = (pthread_t *)malloc(sizeof(pthread_t) * pool_size);

    if (threads == NULL)
    {
        free(pool);
        return NULL;
    }

    pool->num_threads = pool_size;
    pool->threads = threads;

    if (pthread_mutex_init(&pool->lock, NULL) == -1)
        DIE("pthread_mutex_init");

    if (sem_init(&pool->empty, 0, 0))
        DIE("sem_init (empty)");

    if (sem_init(&pool->full, 0, pool_size))
        DIE("sem_init (full)");

    return pool;
}


