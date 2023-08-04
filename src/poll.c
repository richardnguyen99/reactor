#include "poll.h"

struct reactor_event *
revent_new(int epoll_fd, int fd)
{
    struct reactor_event *rev = malloc(sizeof(struct reactor_event));

    if (rev == NULL)
        return NULL;

    rev->fd       = fd;
    rev->epoll_fd = epoll_fd;

    rev->req = NULL;
    rev->res = NULL;

    rev->raw = (char *)malloc(BUFSIZ);

    return rev;
}

int
revent_add(struct reactor_event *rev)
{
    struct epoll_event ev;
    ev.data.ptr = rev;
    ev.events   = EPOLLIN | EPOLLET;

    if (epoll_ctl(rev->epoll_fd, EPOLL_CTL_ADD, rev->fd, &ev) == ERROR)
        return ERROR;

    return SUCCESS;
}

int
revent_mod(struct reactor_event *rev, int flags)
{
    struct epoll_event ev;
    ev.data.ptr = rev;
    ev.events   = flags | EPOLLET;

    if (epoll_ctl(rev->epoll_fd, EPOLL_CTL_MOD, rev->fd, &ev) == -1)
        return ERROR;

    return SUCCESS;
}

int
revent_destroy(struct reactor_event *rev)
{
    if (rev == NULL)
        return SUCCESS;

    if (epoll_ctl(rev->epoll_fd, EPOLL_CTL_DEL, rev->fd, NULL) == ERROR)
        return ERROR;

    if (close(rev->fd) == ERROR)
        return ERROR;

    rev->fd = -1;

    printf("Closed fd: %d\n", rev->fd);

    if (rev->raw != NULL)
        free(rev->raw);

    if (rev->req != NULL)
        request_free(rev->req);

    if (rev->res != NULL)
        response_free(rev->res);

    free(rev);

    return SUCCESS;
}
