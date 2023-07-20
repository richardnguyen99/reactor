// Private methods for reactor.c

#include "reactor.h"

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

int
_set_nonblocking(int fd)
{
    int flags, status;

    // Get the current flags
    flags = fcntl(fd, F_GETFL, 0);

    if (flags == -1)
        return ERROR;

    // Append the non-blocking flag to the current flags
    flags |= O_NONBLOCK;
    status = fcntl(fd, F_SETFL, flags);

    if (status == -1)
        return ERROR;

    return SUCCESS;
}

struct reactor_event *
_init_event(int fd)
{
    void *mem                 = malloc(sizeof(struct reactor_event));
    struct reactor_event *rev = (struct reactor_event *)mem;

    if (rev == NULL)
        return NULL;

    rev->fd   = fd;
    rev->body = NULL;
    rev->len  = 0;

    rev->req = (struct request *)malloc(sizeof(struct request));
    rev->res = (struct response *)malloc(sizeof(struct response));
    rev->raw = malloc(BUFSIZ);

    if (rev->req == NULL || rev->res == NULL || rev->raw == NULL)
        goto safe_exit;

    return rev;

safe_exit:
    if (rev->raw)
    {
        free(rev->raw);
        rev->raw = NULL;
    }

    if (rev->req)
    {
        free(rev->req);
        rev->req = NULL;
    }

    if (rev->res)
    {
        free(rev->res);
        rev->res = NULL;
    }

    if (rev != NULL)
    {
        free(rev);
        rev = NULL;
    }

    return NULL;
}
