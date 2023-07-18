#include "reactor.h"

int
_prepare_socket(char *host, const char *service);

// =============================================================================

struct reactor *
reactor_init(int argc, char *argv[])
{
    struct reactor *server = malloc(sizeof(struct reactor));

    if (server == NULL)
        DIE("(reactor_init) malloc");

    memset(&(server->port), '\0', sizeof(server->port));
    memset(server->port.str, '\0', sizeof(char) * PRTSIZ);

    memset(server->ip, '\0', INET_ADDRSTRLEN);

    server->port.number = 9999;
    server->server_fd   = -1;

    debug("Initialize reactor instance\n");

    return server;
}

int
reactor_load(struct reactor *server)
{
    int status = SUCCESS;

    server->server_fd = _prepare_socket(server->ip, "9999");

    return status;
}

int
reactor_boot(struct reactor *server)
{
    int status = SUCCESS;

    if ((status = listen(server->server_fd, SOMAXCONN) == ERROR))
        goto safe_exit;

safe_exit:
    return status;
}

void
reactor_run(struct reactor *server)
{
    int fd = -1;
    ssize_t nread;
    size_t total;
    char buf[BUFSIZ];

    for (;;)
    {
        fd = accept(server->server_fd, NULL, NULL);

        if (fd == -1)
            DIE("(reactor_run) accept");

        debug("Accepted connection on fd %d\n", fd);

        for (;;)
        {
            nread = 0;
            total = 0;
            memset(buf, '\0', BUFSIZ);

            for (;;)
            {
#ifndef MSG_NO_FLAG
#define MSG_NO_FLAG 0
#endif
                nread = recv(fd, buf + total, 1, MSG_NO_FLAG);

                if (nread == -1)
                    DIE("(reactor_run) recv");

                if (nread == 0)
                    break;

                total += nread;

                if (buf[total - 2] == '\r' && buf[total - 1] == '\n')
                    break;
            }

            if (total == 2 && buf[0] == '\r' && buf[1] == '\n')
                break;

            debug("Buffer:\t%s", buf);
            debug("Received %zu bytes\n", total);
        }

        if (close(fd) == ERROR)
            DIE("(reactor_run) close");
    }

    return;
}

void
reactor_destroy(struct reactor *server)
{
    if (server->server_fd != -1)
        close(server->server_fd);

    free(server);
}

// =============================================================================

int
_prepare_socket(char *host, const char *service)
{
    struct addrinfo hints, *results, *rp;
    int status, fd;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = AI_PASSIVE;

    status = getaddrinfo(NULL, service, &hints, &results);

    if (status != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(EXIT_FAILURE);
    }

    for (rp = results; rp != NULL; rp = rp->ai_next)
    {
        fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

        if (fd == -1)
            continue;

        status =
            setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

        if (status == -1)
        {
            close(fd);
            continue;
        }

        if (bind(fd, rp->ai_addr, rp->ai_addrlen) == 0)
            break;

        close(fd);
    }

    if (rp == NULL)
        DIE("(prepare_socket) bind");

    inet_ntop(rp->ai_family, &((struct sockaddr_in *)rp->ai_addr)->sin_addr,
              host, INET_ADDRSTRLEN);

safe_exit:
    freeaddrinfo(results);
    return fd;
}
