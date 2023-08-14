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

static rx_status_t
rx_daemon_init_socket(struct rx_daemon *d)
{
    rx_status_t       status;
    struct rx_socket *s;

    status = RX_OK;

    s = rx_socket_create();

    if (s == NULL)
    {
        fprintf(stderr, "\
reactor: [error] rx_daemon_init_socket: rx_socket_create() failed\n\
");

        return RX_ERROR;
    }

    status = rx_socket_prepare_listening(s);

    if (status != RX_OK)
    {
        fprintf(stderr, "\
reactor: [error] rx_daemon_init_socket: rx_socket_prepare_listening() failed\n\
");

        return RX_ERROR;
    }

    status = rx_socket_listen(s);
    if (status != RX_OK)
    {
        fprintf(stderr, "\
reactor: [error] rx_daemon_init_socket: rx_socket_listen() failed\n\
");

        return RX_ERROR;
    }

    d->sock = s;

    return status;
}

rx_status_t
rx_daemon_init(struct rx_daemon *d, int argc, char *const *argv)
{
    rx_status_t status;

    status = RX_OK;

    /* TODO: Add configuration from argc and argv */

    d->pool_size       = RX_MAX_THREADS;
    d->backlog         = RX_MAX_BACKLOG;
    d->max_events      = RX_MAX_EVENTS;
    d->max_threads     = RX_MAX_THREADS;
    d->max_connections = RX_MAX_CONNECTIONS;

    d->pool = NULL;

    return status;
}

rx_status_t
rx_daemon_bootstrap(struct rx_daemon *d)
{
    rx_status_t status;
    u_char     *reason;

    status = rx_daemon_init_socket(d);

    if (status != RX_OK)
    {
        reason = "rx_daemon_init_socket() failed";
        goto error;
    }

    return status;

error:
    fprintf(stderr, "\
reactor: [error] rx_daemon_bootstrap: %s\n\
",
            reason);

    return status;
}
