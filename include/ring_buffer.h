/**
 * @file ring_buffer.h
 * @author Richard Nguyen (richard@richardhnguyen.com)
 * @brief Ring buffer header for bounded buffer
 * @version 0.1
 * @date 2023-07-22
 *
 * @copyright Copyright (c) 2023
 */

#ifndef _REACTOR_RING_BUFFER_H_
#define _REACTOR_RING_BUFFER_H_ 1

#include "defs.h"
#include "poll.h"

struct ring_buffer
{
    /* Maximum capacity of this ring buffer */
    size_t cap;

    /* Current number of items in this ring buffer */
    size_t size;

    /* Index position to enqueue a new fd*/
    size_t in;

    /* Index position to dequeue an existing fd */
    size_t out;

    /* Array of reactor events */
    struct reactor_event **events;
};

struct ring_buffer *
rbuffer_new(size_t cap);

size_t
rbuffer_append(struct ring_buffer *buffer, struct reactor_event *rev);

struct reactor_event *
rbuffer_pop(struct ring_buffer *buffer);

void
rbuffer_free(struct ring_buffer *buffer);

#endif // _REACTOR_RING_BUFFER_H_
