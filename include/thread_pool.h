/**
 * @file thread_pool.h
 * @author Richard Nguyen (richard@richardhnguyen.com)
 * @brief Thread pool header file
 * @version 0.1
 * @date 2023-07-15
 * 
 * @copyright Copyright (c) 2023
 */

#ifndef _THREAD_POOL_H
#define _THREAD_POOL_H 1

#include "defs.h"
#include "ring_buffer.h"

struct thread_pool
{
    /*  Number of threads in the pool */
    size_t num_threads;

    /* Pool of POSIX threads */
    pthread_t *threads;

    /* Mutex lock to protect the critical section */
    pthread_mutex_t lock;

    /* Synchronization primitive to check if the task queue is full */
    sem_t full;

    /* Synchronization primitive to check if the task queue is empty */
    sem_t empty;
};

struct thread_args
{
    struct thread_pool *pool;
    struct queue *queue;
};

struct thread_pool *tp_create(size_t queue_cap, size_t pool_size);

#endif // _THREAD_POOL_H
