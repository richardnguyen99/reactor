#include "reactor.h"

extern int
_set_nonblocking(int fd);

void
__reactor_accept(struct reactor *server, struct reactor_event *rev)
{
    int fd, ret;
    struct reactor_socket *rsk;
    struct reactor_timer *rtm;
    struct reactor_event *rev_timer;
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);

    fd = accept(server->server_fd, (struct sockaddr *)&addr, &len);

    if (fd == -1)
        DIE("(reactor_run) accept");

    if (_set_nonblocking(fd) == ERROR)
        DIE("(reactor_run) _set_nonblocking");

    // Create a socket event
    rsk = rsocket_new(server->epollfd, fd);
    if (rsk == NULL)
        DIE("(reactor_run) rsocket_new");

    // Create a timer event
    rtm = rtimer_new(server->epollfd, rev);
    if (rtm == NULL)
        DIE("(reactor_run) rtimer_new");

    // Create a generic event for the timer
    rev_timer = revent_new(server->epollfd, EVENT_TIMER);
    if (rev_timer == NULL)
        DIE("(__reactor_run) revent_new(rtm)");

    rev_timer->data.rtm = rtm;

    rsk->rev_timer = rev_timer;
    rsk->client    = addr;

    rev->data.rsk = rsk;
    rev->flag     = EVENT_SOCKET;

    ret = revent_add(rev);
    if (ret == ERROR)
        DIE("(__reactor_run) revent_add(rsk)");

    ret = revent_add(rev_timer);
    if (ret == ERROR)
        DIE("(__reactor_run) revent_add(rtm)");
}