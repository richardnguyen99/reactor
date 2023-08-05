#include "poll.h"

struct reactor_socket *
rsocket_new(int epoll_fd, int fd)
{
    struct reactor_socket *rev =
        (struct reactor_socket *)malloc(sizeof(struct reactor_socket));

    if (rev == NULL)
        return NULL;

    rev->fd       = fd;
    rev->epoll_fd = epoll_fd;

    rev->req = NULL;
    rev->res = NULL;
    rev->rtm = NULL;

    pthread_rwlock_init(&(rev->req_lock), NULL);
    pthread_rwlock_init(&(rev->res_lock), NULL);

    return rev;
}

struct reactor_timer *
rtimer_new(int epoll_fd, int fd, int timeout)
{
    struct reactor_timer *rtm =
        (struct reactor_timer *)malloc(sizeof(struct reactor_timer));

    if (rtm == NULL)
        return NULL;

    rtm->fd       = fd;
    rtm->epoll_fd = epoll_fd;
    rtm->timeout  = timeout;

    return rtm;
}

int
rsocket_add(struct reactor_socket *rev)
{
    struct epoll_event ev;
    ev.data.ptr = rev;
    ev.events   = EPOLLIN | EPOLLET;

    if (epoll_ctl(rev->epoll_fd, EPOLL_CTL_ADD, rev->fd, &ev) == ERROR)
        return ERROR;

    return SUCCESS;
}

int
rtimer_add(struct reactor_timer *rtm)
{
    struct epoll_event ev;
    ev.data.ptr = rtm;
    ev.events   = EPOLLIN | EPOLLET;

    if (epoll_ctl(rtm->epoll_fd, EPOLL_CTL_ADD, rtm->fd, &ev) == ERROR)
        return ERROR;

    return SUCCESS;
}

int
rsocket_mod(struct reactor_socket *rev, int flags)
{
    struct epoll_event ev;
    ev.data.ptr = rev;
    ev.events   = flags | EPOLLET;

    if (epoll_ctl(rev->epoll_fd, EPOLL_CTL_MOD, rev->fd, &ev) == -1)
        return ERROR;

    return SUCCESS;
}

int
rsocket_destroy(struct reactor_socket *rev)
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
    {
        free(rev->raw);
        rev->raw = NULL;
    }

    if (rev->req != NULL)
    {
        request_free(rev->req);
        rev->req = NULL;
    }

    if (rev->res != NULL)
    {
        response_free(rev->res);
        rev->res = NULL;
    }

    pthread_rwlock_destroy(&(rev->req_lock));
    pthread_rwlock_destroy(&(rev->res_lock));

    free(rev);

    return SUCCESS;
}
