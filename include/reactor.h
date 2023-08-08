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
#include "http.h"
#include "poll.h"
#include "threads.h"
#include "util.h"

/* Do not use by itself! */
struct __port
{
    /* Integer representation of the port number */
    int16_t number;

    /* String representation of the port number */
    char str[6];
};

typedef struct __port port_t;

struct reactor
{
    /* Port number of the server */
    port_t port;

    /* Listening socket of the server */
    int server_fd;

    /* File descriptor of the epoll interface */
    int epollfd;

    /* Array of epoll events */
    struct epoll_event events[MAX_EVENTS];

    /* IP address of the server */
    char ip[INET_ADDRSTRLEN];

    /* Ring buffer (bounded buffer) for the task queue */
    struct thread_pool *pool;
};

/**
 * @brief Initialize a new reactor server instance based on the CLI arguments.
 *
 * @param argc Number of arguments (from the main function)
 * @param argv Array of arguments (from the main function)
 * @return {struct reactor*} Pointer to the reactor server instance
 */
void
reactor_init(struct reactor **server, int argc, char *argv[]);

/**
 * @brief Load the server configuration into the server instance.
 *
 * @param server A pointer to the server instance
 */
void
reactor_load(struct reactor *server);

/**
 * @brief Prepare and boot up the server instance.
 *
 * @param server A pointer to the server instance
 * @return {int} On success, the function returns 0 (SUCCESS in macro). On
 * error, -1 (ERROR in macro) is returned.
 */
int
reactor_boot(struct reactor *server);

/**
 * @brief Main loop of the server instance
 *
 * @param server A pointer to the server instance
 */
int
reactor_run(struct reactor *server);

/**
 * @brief  Clean up the server on exiting
 *
 * @param server A pointer to the server isntance
 */
void
reactor_destroy(struct reactor *server);

#endif // _REACTOR_H_
