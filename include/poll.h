/**
 * @file poll.h
 * @author Richard Nguyen (richard@richardhnguyen.com)
 * @brief Polling for events header file
 * @version 0.1
 * @date 2023-07-21
 *
 * @copyright Copyright (c) 2023
 */

#ifndef _REACTOR_POLL_H_
#define _REACTOR_POLL_H_ 1

#include "defs.h"
#include "request.h"
#include "response.h"

struct reactor_socket
{
    int fd;
    int epoll_fd;
    char *raw;
    uint64_t len;

    struct sockaddr_in client;

    struct request *req;
    struct response *res;

    struct reactor_event *rev_timer;
    struct reactor_event *rev_cnnct;

    /* Write lock since response is accessed by multiple threads */
    pthread_rwlock_t res_lock;

    pthread_rwlock_t req_lock; /* Not sure if this is needed since only one
                  thread (the main thread) is writing to the request */
};

struct reactor_timer
{
    int fd;
    int epoll_fd;
    int timeout;

    struct reactor_event *rev_socket;
};

struct reactor_connection
{
    int fd;
    int epoll_fd;
    int timeout;

    struct reactor_event *rev_socket;
};

typedef enum event_flag
{
    EVENT_SOCKET = 1 << 0,
    EVENT_TIMER  = 1 << 1,
    EVENT_CNNCT  = 1 << 2,

} evflag_t;

typedef union event_pointer
{
    struct reactor_socket *rsk;
    struct reactor_timer *rtm;
    struct reactor_connection *rcn;
} evptr_t;

struct reactor_event
{
    int flag;
    evptr_t data;

    size_t __refcnt;
};

struct reactor_socket *
rsocket_new(int epoll_fd, int fd);

int
rsocket_add(struct reactor_socket *rev);

int
rsocket_mod(struct reactor_socket *rev, int flags);

int
rsocket_destroy(struct reactor_socket *rev);

struct reactor_timer *
rtimer_new(int epoll_fd, struct reactor_event *rev_socket);

int
rtimer_add(struct reactor_timer *rtm);

int
rtimer_mod(struct reactor_timer *rtm, int timeout_sec);

int
rtimer_destroy(struct reactor_timer *rtm);

struct reactor_event *
revent_new(int epoll_fd, evflag_t flag);

int
revent_add(struct reactor_event *rev);

int
revent_mod(struct reactor_event *rev, int flags);

int
revent_destroy(struct reactor_event *rev);

#endif // _REACTOR_POLL_H_
