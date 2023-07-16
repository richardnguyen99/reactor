#include "ring_buffer.h"

struct queue *queue_init(size_t cap)
{
    struct queue *q = malloc(sizeof(struct queue));

    if (q == NULL)
        return NULL;

    q->cap = cap;
    q->size = 0;
    q->in = 0;
    q->out = 0;
    q->fds = malloc(sizeof(int) * cap);

    if (q->fds == NULL)
    {
        free(q);
        return NULL;
    }

    for (size_t i = 0; i < cap; i++)
        q->fds[i] = -1;

    return q;
}

void queue_push_back(struct queue *q, int fd)
{
    q->fds[q->in] = fd;
    q->in = (q->in + 1) % q->cap;
    q->size++;
}

int queue_pop_front(struct queue *q)
{
    size_t i = q->out;
    int fd = q->fds[i];

    q->fds[i] = -1;
    q->out = (i + 1) % q->cap;
    q->size--;

    return fd;
}
