/**
 * @file rx_daemon.h
 * @author Richard Nguyen (richard@richardhnguyen.com)
 * @brief Header for daemon module that runs the reactor project
 * @version 0.1
 * @date 2023-08-12
 *
 * @copyright Copyright (c) 2023
 */

#ifndef _RX_DAEMON_H_
#define _RX_DAEMON_H_ 1

#include <rx_defs.h>
#include <rx_core.h>

struct rx_daemon
{
    int                    efd;                /* epoll instance fd*/
    int                    lfd;                /* liste*/
    struct sockaddr_in     host;               /* host addr info */

    size_t                 pool_size;          /* thread pool size */
    size_t                 backlog;            /* listening backlog */
    size_t                 max_events;         /* max events for polling */
    size_t                 max_threads;        /* max supported threads */
    size_t                 max_connections;    /* max supported connections */
    size_t                 connections;        /* current connections */

    struct epoll_event     evs[RX_MAX_EVENTS]; /* epoll event structure */
    struct rx_socket      *sock;               /* socket instance */
    struct rx_thread_pool *pool;               /* thread pools */
};

rx_status_t
rx_daemon_init(struct rx_daemon *d, int argc, char *const *argv);

rx_status_t
rx_daemon_bootstrap(struct rx_daemon *d);

#endif /* _RX_DAEMON_H_ */
