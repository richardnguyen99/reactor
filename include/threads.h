/**
 * @file threads.h
 * @author Richard Nguyen (richard@richardhnguyen.com)
 * @brief Thread pool implementation
 * @version 0.1
 * @date 2023-07-22
 *
 * @copyright Copyright (c) 2023
 */

#ifndef _REACTOR_THREADS_H_
#define _REACTOR_THREADS_H_ 1

#include "defs.h"
#include "poll.h"
#include "ring_buffer.h"

typedef void *(*thread_func)(void *arg);

struct http_obj;
struct reactor_event;
struct reactor;

struct thread_pool
{
    /* Number of threads in this thread pool */
    size_t size;

    /* Array of threads */
    pthread_t *threads;

    /* Mutex lock for critical section such as bounded buffer of sockets */
    pthread_mutex_t lock;

    /* semaphore for controling if a buffer is full and the master thread
     needs to wait for some available spaces  */
    sem_t full;

    /* semaphore for controling if a buffer is empty and all working threads
     need to wait for the master thread to produce some work */
    sem_t empty;

    /* Array of tasks */
    struct ring_buffer *buffer;

    /* Function pointer to the thread function */
    thread_func func;
};

/* Task object used by the main thread (event loop) to produce a task for worker
 * threads to work. A task object must be pushed into a blocking queue. */
struct thread_task
{
    /* Reference to the server instance. Information about server can be found
     * here */
    struct reactor *server;

    /* Reference to the event and the client instance. Information about the
     * server can be found here */
    struct reactor_event *rev;
};

struct thread_task *
thread_task_new(struct reactor *server, struct reactor_event *rev);

struct thread_pool *
pool_new(size_t no_threads, size_t buf_size, thread_func func);

int
pool_init(struct thread_pool *pool);

void
pool_free(struct thread_pool *pool);

#endif // _REACTOR_THREADS_H_
