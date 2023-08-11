#include "reactor.h"
#include "threads.h"

void
__reactor_in(struct reactor *server, struct reactor_event *rev)
{
    if (rev->state == 1)
        return;

    u_char

        debug("\
===================================EPOLLIN======================================\n\
Client: %s:%d\n\
Socket: %d\n\
\n",
              inet_ntoa(rev->data.rsk->client.sin_addr),
              ntohs(rev->data.rsk->client.sin_port), rev->data.rsk->fd);

    if (rev->refcnt > 5)
    {
        debug("(__reactor_in) refcnt > 5\n");
        return;
    }

    struct thread_task *task;

    task = thread_task_new(server, rev);
    if (task == NULL)
        DIE("(__reactor_in) thread_task_new");

    // Start to put the task to the queue for thread pool
    if (sem_wait(&(server->pool->empty)) == ERROR)
        DIE("(reactor_run) sem_wait");
    if (pthread_mutex_lock(&(server->pool->lock)) == ERROR)
        DIE("(reactor_run) pthread_mutex_lock");

    if (rbuffer_append(server->pool->buffer, task) == ERROR)
        DIE("(reactor_run) rbuffer_push");

    if (pthread_mutex_unlock(&(server->pool->lock)) == ERROR)
        DIE("(reactor_run) pthread_mutex_unlock");
    if (sem_post(&(server->pool->full)) == ERROR)
        DIE("(reactor_run) sem_post");

    return;
}
