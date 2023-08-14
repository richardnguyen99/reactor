/**
 * @file rx_socket.c
 * @author Richard Nguyen (richard@richardhnguyen.com)
 * @brief Implementation for socket module.
 * @version 0.1
 * @date 2023-08-13
 *
 * @copyright Copyright (c) 2023
 *
 */

#include <rx_defs.h>
#include <rx_core.h>

static struct rx_addr *
rx_socket_gai(const char *host, const char *port)
{
    struct rx_addr  hints;
    struct rx_addr *res;

    memset(&hints, 0, sizeof(struct rx_addr));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = AI_PASSIVE;

    if (getaddrinfo(host, port, &hints, &res) != 0)
    {
        fprintf(stderr, "\
reactor: [error] rx_socket_gai: getaddrinfo(%s, %s, %p, %p) failed\n\
",
                host, port, &hints, &res);
        return NULL;
    }

    return res;
}

static void
rx_socket_fai(struct rx_addr **addr)
{
    if (*addr == NULL)
        return;

    freeaddrinfo(*addr);

    fprintf(stdout, "\
reactor: [info] rx_socket_fai: freeaddrinfo(%p) success\n\
    ",
            *addr);

    *addr = NULL;
}

static rx_status_t
rx_socket_bind(struct rx_addr *addr, int fd)
{
    rx_status_t     status;
    struct rx_addr *rp;

    rp = addr;

    status = RX_OK;

    if (bind(fd, rp->ai_addr, rp->ai_addrlen) == 0)
    {
        fprintf(stdout, "\
reactor: [info] rx_socket_bind: bind(%d, %p, %ld) success\n\
",
                fd, rp->ai_addr, rp->ai_addrlen);

        return RX_OK;
    }

    fprintf(stderr, "\
reactor: [error] rx_socket_bind: bind(%d, %p, %ld) failed\n\
",
            fd, rp->ai_addr, rp->ai_addrlen);

    status = RX_ERROR;

    return status;
}

// TODO: add configurable host and port

struct rx_socket *
rx_socket_create()
{
    struct rx_socket *s;
    struct rx_addr   *addr;

    s = rx_calloc(1, sizeof(struct rx_socket));

    if (s == NULL)
    {
        fprintf(stderr, "\
reactor: [error] rx_socket_create: rx_calloc(%ld, %ld) failed\n\
",
                1, sizeof(struct rx_socket));

        return NULL;
    }

    s->fd       = -1;
    s->domain   = AF_UNSPEC;
    s->type     = SOCK_STREAM;
    s->protocol = 0;
    s->blocking = 1;

    return s;
}

rx_status_t
rx_socket_prepare_listening(struct rx_socket *s)
{
    rx_status_t         status;
    struct rx_addr     *addr, *rp;
    char                port[7];
    struct sockaddr_in *sin;
    int                 fd;

    status = RX_UNSET;

    sin = &s->addr;

    (void)rx_itoa((int)sin->sin_port, port);
    addr = rx_socket_gai(NULL, port);

    for (rp = addr; rp != NULL && status != RX_OK; rp = rp->ai_next)
    {
        fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

        if (fd == -1)
        {
            fprintf(stderr, "\
reactor: [error] rx_socket_bind: socket(%d, %d, %d) failed\n\
",
                    rp->ai_family, rp->ai_socktype, rp->ai_protocol);

            status = RX_ERROR;
            break;
        }

        fprintf(stdout, "\
reactor: [info] rx_socket_bind: socket(%d, %d, %d) success\n\
",
                rp->ai_family, rp->ai_socktype, rp->ai_protocol);

        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) ==
            -1)
        {
            fprintf(stderr, "\
reactor: [error] rx_socket_bind: setsockopt(%d, %d, %d, %p, %ld) failed\n\
",
                    fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

            status = RX_ERROR;
            break;
        }

        status = rx_socket_bind(rp, fd);
    }

    if (rp == NULL)
    {
        fprintf(stderr, "\
reactor: [error] rx_socket_bind: could not bind to any address\n\
");

        status = RX_ERROR;
    }

    rx_socket_fai(&addr);

    s->fd       = fd;
    s->domain   = rp->ai_family;
    s->type     = rp->ai_socktype;
    s->protocol = rp->ai_protocol;
    s->blocking = 0;

    return status;
}

rx_status_t
rx_socket_listen(struct rx_socket *s)
{
    rx_status_t status;

    status = RX_UNSET;

    if (listen(s->fd, RX_MAX_BACKLOG) == -1)
    {
        fprintf(stderr, "\
reactor: [error] rx_socket_listen: listen(%d, %d) failed\n\
",
                s->fd, RX_MAX_BACKLOG);

        status = RX_ERROR;
    }

    fprintf(stdout, "\
reactor: [info] rx_socket_listen: listen(%d, %d) success\n\
",
            s->fd, RX_MAX_BACKLOG);

    status = RX_OK;

    return status;
}
