/* MIT License
 *
 * Copyright (c) 2023 Richard H. Nguyen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <rx_config.h>
#include <rx_core.h>

void *
rx_thread_pool_worker(void *arg)
{
    struct rx_thread_pool *pool = arg;
    pthread_t tid               = pthread_self();

    for (;;)
    {
        sem_wait(&pool->full);
        pthread_mutex_lock(&pool->lock);

        struct rx_task *task = rx_ring_pop(pool->ring);

        pthread_mutex_unlock(&pool->lock);
        sem_post(&pool->empty);

        if (task == NULL)
        {
            continue;
        }

        task->handle(task->arg);

        struct epoll_event event;
        struct rx_connection *conn = task->arg;
        int ret;

        conn->state = conn->state == RX_CONNECTION_STATE_CLOSING
                          ? RX_CONNECTION_STATE_CLOSING
                          : RX_CONNECTION_STATE_WRITING_RESPONSE;

        if (conn->state == RX_CONNECTION_STATE_CLOSING)
        {
            event.events = EPOLLERR | EPOLLRDHUP | EPOLLET;
        }
        else
        {
            event.events = EPOLLOUT | EPOLLET;
        }

        event.data.ptr = conn;

        ret = epoll_ctl(conn->efd, EPOLL_CTL_MOD, conn->fd, &event);

        if (ret == -1)
        {
            rx_log(
                LOG_LEVEL_0, LOG_TYPE_ERROR, "epoll_ctl: %s\n", strerror(errno)
            );
            return RX_ERROR_PTR;
        }

        rx_log(
            LOG_LEVEL_0, LOG_TYPE_INFO,
            "[Thread %ld]%4.sSubmit finished task to event poll\n", tid, ""
        );

        free(task);
    }
}

int
rx_thread_pool_init(struct rx_thread_pool *pool, struct rx_ring *ring)
{
    int ret;

    pool->nthreads = 8;
    pool->ring     = ring;

    ret = pthread_mutex_init(&pool->lock, NULL);

    if (ret == -1)
    {
        rx_log(
            LOG_LEVEL_0, LOG_TYPE_ERROR, "pthread_mutex_init: %s\n",
            strerror(errno)
        );
        return RX_ERROR;
    }

    ret = sem_init(&pool->empty, 0, 0);

    if (ret == -1)
    {
        rx_log(LOG_LEVEL_0, LOG_TYPE_ERROR, "sem_init: %s\n", strerror(errno));
        return RX_ERROR;
    }

    ret = sem_init(&pool->full, 0, 256);

    if (ret == -1)
    {
        rx_log(LOG_LEVEL_0, LOG_TYPE_ERROR, "sem_init: %s\n", strerror(errno));
        return RX_ERROR;
    }

    for (size_t i = 0; i < 8; i++)
    {
        ret = pthread_create(
            &pool->threads[i], NULL, rx_thread_pool_worker, pool
        );

        if (ret != 0)
        {
            rx_log(
                LOG_LEVEL_0, LOG_TYPE_ERROR, "pthread_create: %s\n",
                strerror(errno)
            );
            pool->nthreads--;
        }
    }

    return RX_OK;
}

int
rx_thread_pool_submit(struct rx_thread_pool *pool, struct rx_task *task)
{
    int ret;

    ret = sem_wait(&pool->empty);

    if (ret == -1)
    {
        rx_log(LOG_LEVEL_0, LOG_TYPE_ERROR, "sem_wait: %s\n", strerror(errno));
        return RX_ERROR;
    }

    ret = pthread_mutex_lock(&pool->lock);

    if (ret == -1)
    {
        rx_log(
            LOG_LEVEL_0, LOG_TYPE_ERROR, "pthread_mutex_lock: %s\n",
            strerror(errno)
        );
        return RX_ERROR;
    }

    rx_ring_push(pool->ring, task);

    ret = pthread_mutex_unlock(&pool->lock);

    if (ret == -1)
    {
        rx_log(
            LOG_LEVEL_0, LOG_TYPE_ERROR, "pthread_mutex_unlock: %s\n",
            strerror(errno)
        );
        return RX_ERROR;
    }

    ret = sem_post(&pool->full);

    if (ret == -1)
    {
        rx_log(LOG_LEVEL_0, LOG_TYPE_ERROR, "sem_post: %s\n", strerror(errno));
        return RX_ERROR;
    }

    rx_log(LOG_LEVEL_0, LOG_TYPE_DEBUG, "Submit task to thread pool\n");

    return RX_OK;
}
