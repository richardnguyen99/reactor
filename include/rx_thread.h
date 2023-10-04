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

#ifndef __RX_THREAD_H__
#define __RX_THREAD_H__ 1

#include <rx_config.h>
#include <rx_core.h>

struct rx_thread_pool
{
    pthread_t threads[8];
    size_t nthreads;

    pthread_mutex_t lock;
    sem_t empty;
    sem_t full;

    struct rx_ring *ring;
};

void *
rx_thread_pool_worker(void *arg);

int
rx_thread_pool_init(struct rx_thread_pool *pool, struct rx_ring *ring);

int
rx_thread_pool_submit(struct rx_thread_pool *pool, struct rx_task *task);

#endif /*  __RX_THREAD_H__ */
