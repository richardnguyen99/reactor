/**
 * @file rx_daemon.c
 * @author Richard Nguyen (richard@richardhnguyen.com)
 * @brief Implementation for daemon module that runs the reactor server.
 * @version 0.1
 * @date 2023-08-12
 *
 * @copyright Copyright (c) 2023
 */

#include <rx_defs.h>
#include <rx_core.h>

static struct rx_daemon *
rx_daemon_create(void);

rx_status_t
rx_daemon_init(struct rx_daemon *d, int argc, char *const *argv)
{
    rx_status_t status = RX_OK;

    /* TODO: Add configuration from argc and argv */

    d->pool_size       = RX_MAX_THREADS;
    d->backlog         = RX_MAX_BACKLOG;
    d->max_events      = RX_MAX_EVENTS;
    d->max_threads     = RX_MAX_THREADS;
    d->max_connections = RX_MAX_CONNECTIONS;

    d->pool = NULL;

    return status;
}

static struct rx_daemon *
rx_daemon_create(void)
{
}
