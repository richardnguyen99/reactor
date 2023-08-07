#include "poll.h"

static inline void
__get_evt_fd(struct reactor_event *rev, int *fd, int *epoll_fd)
{

    if (rev->flag == EVENT_SOCKET)
    {
        *fd       = rev->data.rsk->fd;
        *epoll_fd = rev->data.rsk->epoll_fd;
    }
    else if (rev->flag == EVENT_TIMER)
    {
        *fd       = rev->data.rtm->fd;
        *epoll_fd = rev->data.rtm->epoll_fd;
    }
}

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

    pthread_rwlock_init(&(rev->req_lock), NULL);
    pthread_rwlock_init(&(rev->res_lock), NULL);

    return rev;
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

struct reactor_timer *
rtimer_new(int epoll_fd, struct reactor_event *rsk)
{
    int timer_fd;
    struct itimerspec its;
    struct reactor_timer *rtm;

    rtm = (struct reactor_timer *)malloc(sizeof(struct reactor_timer));

    if (rtm == NULL)
        return NULL;

    memset(&its, 0, sizeof(struct itimerspec));
    its.it_value.tv_sec = 10;
    timer_fd            = timerfd_create(CLOCK_MONOTONIC, 0);
    if (timer_fd == -1)
    {
        perror("timerfd_create");
        exit(EXIT_FAILURE);
    }

    timerfd_settime(timer_fd, 0, &its, NULL);

    rtm->fd         = timer_fd;
    rtm->epoll_fd   = epoll_fd;
    rtm->timeout    = 10;
    rtm->rev_socket = rsk;

    return rtm;
}

int
rimter_add(struct reactor_timer *rtm)
{
    struct epoll_event ev;
    ev.data.ptr = rtm;
    ev.events   = EPOLLIN | EPOLLET;

    if (epoll_ctl(rtm->epoll_fd, EPOLL_CTL_ADD, rtm->fd, &ev) == ERROR)
        return ERROR;

    return SUCCESS;
}

int
rtimer_mod(struct reactor_timer *rtm, int timeout_sec)
{
    struct itimerspec its;
    memset(&its, 0, sizeof(struct itimerspec));
    its.it_value.tv_sec  = timeout_sec;
    its.it_value.tv_nsec = 0;

    if (timerfd_settime(rtm->fd, 0, &its, NULL) == -1)
        return ERROR;

    return SUCCESS;
}

int
rtimer_destroy(struct reactor_timer *rtm)
{
    if (rtm == NULL)
        return SUCCESS;

    if (epoll_ctl(rtm->epoll_fd, EPOLL_CTL_DEL, rtm->fd, NULL) == ERROR)
        return ERROR;

    if (close(rtm->fd) == ERROR)
        return ERROR;

    free(rtm);

    return SUCCESS;
}

struct reactor_event *
revent_new(int epoll_fd, evflag_t flag)
{
    struct reactor_event *rev =
        (struct reactor_event *)malloc(sizeof(struct reactor_event));

    if (rev == NULL)
        return NULL;

    rev->flag = flag;
    memset(&(rev->data), 0, sizeof(evptr_t));

    rev->__refcnt = 1;

    return rev;
}

int
revent_add(struct reactor_event *rev)
{
    if (rev == NULL)
        return ERROR;

    struct epoll_event ev;
    int epoll_fd, fd;

    ev.data.ptr = rev;
    ev.events   = EPOLLIN | EPOLLET;

    __get_evt_fd(rev, &fd, &epoll_fd);

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) == ERROR)
        return ERROR;

    return SUCCESS;
}

int
revent_mod(struct reactor_event *rev, int flags)
{
    if (rev == NULL)
        return ERROR;

    struct epoll_event ev;
    int epoll_fd, fd;

    ev.data.ptr = rev;
    ev.events   = flags | EPOLLET;

    __get_evt_fd(rev, &fd, &epoll_fd);

    if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev) == ERROR)
        return ERROR;

    return SUCCESS;
}

int
revent_destroy(struct reactor_event *rev)
{
    if (rev == NULL)
        return ERROR;

    if (rev->flag == EVENT_SOCKET)
    {
        rsocket_destroy(rev->data.rsk);
        rev->data.rsk = NULL;

        debug("destroyed socket\n");
    }
    else if (rev->flag == EVENT_TIMER)
    {
        rtimer_destroy(rev->data.rtm);
        rev->data.rtm = NULL;

        printf("destroyed timer\n");
    }

    free(rev);

    return SUCCESS;
}
