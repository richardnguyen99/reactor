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

struct reactor_event
{
    int fd;
    int epoll_fd;
    char *raw;
    uint64_t len;

    struct request *req;
    struct response *res;
};

struct reactor_event *
revent_new(int epoll_fd, int fd);

int
revent_add(struct reactor_event *rev);

int
revent_mod(struct reactor_event *rev, int flags);

int
revent_destroy(struct reactor_event *rev);

#endif // _REACTOR_POLL_H_
