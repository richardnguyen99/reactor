#include "poll.h"

int poll_create(int flags)
{
    int epoll_fd = epoll_create1(flags);

    if (epoll_fd == -1)
        return ERROR;

    return epoll_fd;

}


int poll_add(int poll_fd, int fd, int events)
{
    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;

    int ret = epoll_ctl(poll_fd, EPOLL_CTL_ADD, fd, &ev);
    if(ret == -1)
        return -1;
}

int poll_del(int poll_fd, int fd, int events, struct epoll_event *poll_events)
{
    return 0;
}


int poll_mod(int poll_fd, int fd, int events, struct epoll_event *poll_events)
{
    return 0;
}

int poll_wait(int poll_fd, int timeout, struct epoll_event *events)
{
    return 0;
}
