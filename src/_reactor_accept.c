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

    fd = accept(server->server_fd, NULL, NULL);

    if (fd == -1)
        DIE("(reactor_run) accept");

    printf("Accepted epollfd: %d\n", server->epollfd);
    printf("Accepted fd: %d\n", fd);

    if (_set_nonblocking(fd) == ERROR)
        DIE("(reactor_run) _set_nonblocking");

    rsk = rsocket_new(server->epollfd, fd);
    if (rsk == NULL)
        DIE("(reactor_run) rsocket_new");

    rev->data.rsk = rsk;
    rev->flag     = EVENT_SOCKET;

    rtm = rtimer_new(server->epollfd, rev);
    if (rtm == NULL)
        DIE("(reactor_run) rtimer_new");

    rev_timer = revent_new(server->epollfd, EVENT_TIMER);
    if (rev_timer == NULL)
        DIE("(__reactor_run) revent_new(rtm)");

    rev_timer->data.rtm = rtm;

    ret = revent_add(rev);
    if (ret == ERROR)
        DIE("(__reactor_run) revent_add(rsk)");
    printf("New connection: %p\n", rev);

    ret = revent_add(rev_timer);
    if (ret == ERROR)
        DIE("(__reactor_run) revent_add(rtm)");
    printf("New timer: %p\n", rev_timer);
}
