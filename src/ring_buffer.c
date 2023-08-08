#include "ring_buffer.h"

struct ring_buffer *
rbuffer_new(size_t cap)
{
    void *buf = malloc(sizeof(struct ring_buffer));
    void *tsk = malloc(sizeof(struct thread_task *) * cap);

    if (buf == NULL)
        return NULL;

    struct ring_buffer *buffer = (struct ring_buffer *)buf;

    buffer->cap   = cap;
    buffer->size  = 0;
    buffer->in    = 0;
    buffer->out   = 0;
    buffer->tasks = (struct thread_task **)tsk;

    if (buffer->tasks == NULL)
    {
        free(buffer);
        return NULL;
    }

    for (size_t i = 0; i < cap; i++)
        buffer->tasks[i] = NULL;

    return buffer;
}

size_t
rbuffer_append(struct ring_buffer *buf, struct thread_task *task)
{
    size_t i      = buf->in;
    buf->tasks[i] = task;
    buf->in       = (i + 1) % buf->cap;
    buf->size++;

    return i;
}

struct thread_task *
rbuffer_pop(struct ring_buffer *buf)
{
    size_t i                 = buf->out;
    struct thread_task *task = buf->tasks[i];

    buf->tasks[i] = NULL;
    buf->out      = (i + 1) % buf->cap;
    buf->size--;

    return task;
}

void
rbuffer_free(struct ring_buffer *buf)
{
    free(buf->tasks);
    free(buf);
}
