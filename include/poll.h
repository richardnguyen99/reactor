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

    struct request *req;
    struct response *res;

    pthread_rwlock_t res_lock;
    pthread_rwlock_t req_lock;
};

struct reactor_timer
{
    int fd;
    int epoll_fd;
    int timeout;

    struct reactor_socket *rsk;
};

typedef union event_pointer
{
    struct reactor_socket *rsk;
    struct reactor_timer *rtm;
} evptr_t;

struct reactor_event
{
    int flag;
    evptr_t data;
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
rtimer_new(int epoll_fd, struct reactor_socket *rsk);

int
rtimer_add(struct reactor_timer *rtm);

int
rtimer_destroy(struct reactor_timer *rtm);

#endif // _REACTOR_POLL_H_
