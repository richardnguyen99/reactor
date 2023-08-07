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

    return;
}
