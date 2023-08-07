#include "reactor.h"

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
    debug("epollin: ");
    // debug("socket: %p\n", rsk);

    // if (rsk->req == NULL)
    // rsk->req = request_new();

    // if (rsk->req == NULL)
    // DIE("(reactor_run) request_new");

    // http_status = request_parse(rsk->req, rsk->fd);

    // // Don't continue if the request processing is not ready
    // if (http_status == HTTP_READ_AGAIN)
    // goto wait_to_read;

    // if (http_status == HTTP_ERROR)
    // DIE("(reactor_run) http_request");

    // // Disarm the timer if the request is parsed successfully
    // struct reactor_timer *rtm = rsk->rev_timer->data.rtm;
    // debug("timer: %d\n", rtm->fd);

    // if (rtimer_mod(rtm, 0) == ERROR)
    // DIE("(reactor_run) rtimer_mod");

    // debug("Disarmed timer on socket %p\n", rsk);

    // // Start to put the task to the queue for thread pool
    // if (sem_wait(&(server->pool->empty)) == ERROR)
    // DIE("(reactor_run) sem_wait");
    // if (pthread_mutex_lock(&(server->pool->lock)) == ERROR)
    // DIE("(reactor_run) pthread_mutex_lock");

    // if (rbuffer_append(server->pool->buffer, rev) == ERROR)
    // DIE("(reactor_run) rbuffer_push");

    // if (pthread_mutex_unlock(&(server->pool->lock)) == ERROR)
    // DIE("(reactor_run) pthread_mutex_unlock");
    // if (sem_post(&(server->pool->full)) == ERROR)
    // DIE("(reactor_run) sem_post");

    // rev->__refcnt++;

    return;
}
