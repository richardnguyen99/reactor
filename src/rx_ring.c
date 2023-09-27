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

void
rx_ring_init(struct rx_ring *ring)
{
    ring->size = 0;
    ring->cap  = 256;
    ring->in   = 0;
    ring->out  = 0;

    for (size_t i = 0; i < ring->cap; i++)
    {
        ring->tasks[i] = NULL;
    }
}

void
rx_ring_push(struct rx_ring *ring, struct rx_task *task)
{
    if (ring->size == ring->cap)
    {
        return;
    }

    ring->tasks[ring->in] = task;
    ring->in              = (ring->in + 1) % ring->cap;
    ring->size++;
}

struct rx_task *
rx_ring_pop(struct rx_ring *ring)
{
    if (ring->size == 0)
    {
        return NULL;
    }

    struct rx_task *task   = ring->tasks[ring->out];
    ring->tasks[ring->out] = NULL;
    ring->out              = (ring->out + 1) % ring->cap;
    ring->size--;

    return task;
}
