#include "ring_buffer.h"

struct ring_buffer *
rbuffer_new(size_t cap)
{
    void *buf = malloc(sizeof(struct ring_buffer));

    if (buf == NULL)
        return NULL;

    struct ring_buffer *buffer = (struct ring_buffer *)buf;

    buffer->cap  = cap;
    buffer->size = 0;
    buffer->in   = 0;
    buffer->out  = 0;
    buffer->events =
        (struct reactor_socket **)malloc(sizeof(struct reactor_socket *) * cap);

    if (buffer->events == NULL)
    {
        free(buffer);
        return NULL;
    }

    for (size_t i = 0; i < cap; i++)
        buffer->events[i] = NULL;

    return buffer;
}

size_t
rbuffer_append(struct ring_buffer *buf, struct reactor_socket *rev)
{
    size_t i       = buf->in;
    buf->events[i] = rev;
    buf->in        = (i + 1) % buf->cap;
    buf->size++;

    return i;
}

struct reactor_socket *
rbuffer_pop(struct ring_buffer *buf)
{
    size_t i                   = buf->out;
    struct reactor_socket *rev = buf->events[i];

    buf->events[i] = NULL;
    buf->out       = (i + 1) % buf->cap;
    buf->size--;

    return rev;
}

void
rbuffer_free(struct ring_buffer *buf)
{
    free(buf->events);
    free(buf);
}
