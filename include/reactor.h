/**
 * @file reactor.h
 * @author Richard Nguyen (richard@richardhnguyen.com)
 * @brief Reactor class header file.
 * @version 0.1.3
 * @date 2023-07-17
 *
 * @copyright Copyright (c) 2023
 */

#ifndef _REACTOR_H_
#define _REACTOR_H_ 1

#include "defs.h"
#include "poll.h"
#include "request.h"
#include "response.h"
#include "threads.h"
#include "util.h"

struct __port
{
    int16_t number;
    char str[6];
};

typedef struct __port port_t;

struct reactor
{
    port_t port;
    int server_fd;
    int epollfd;

    struct epoll_event events[MAX_EVENTS];

    char ip[INET_ADDRSTRLEN];

    struct thread_pool *pool;
};

struct reactor *
reactor_init(int argc, char *argv[]);

int
reactor_load(struct reactor *server);

int
reactor_boot(struct reactor *server);

int
reactor_run(struct reactor *server);

void
reactor_destroy(struct reactor *server);

#endif // _REACTOR_H_
