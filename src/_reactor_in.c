#include "reactor.h"
#include "threads.h"

size_t i = 0;

/*
    >>> Function to handle EPOLLIN events, specifically for the server socket

    EPOLLIN events happen when there is data to be read from the socket. The
    epoll interface will monitor and trigger the event. This function will
    handle that event.

    The data read from the socket is an HTTP request from the client socket.
    After receiving the request, the server will construct a work containing
    essential information about the request and the server. The work will be
    added to the work queue for the worker threads to process.

    The structure for the work is `struct request`, which is defined in the
   `src/request.h`.

    ----------------------------------------------------------------------------
    >>> Parameters:
        `struct reactor *server`: The server reactor
        `struct reactor_event *rev`: The event that triggered the callback
 */
void
__reactor_in(struct reactor *server, struct reactor_event *rev)
{
    debug("\
===================================EPOLLIN======================================\n\
Client: %s:%d\n\
Socket: %d\n\
Event: %d\n\
\n",
          inet_ntoa(rev->data.rsk->client.sin_addr),
          ntohs(rev->data.rsk->client.sin_port), rev->data.rsk->fd, (++i));

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
