#include "reactor.h"

int
_prepare_socket(char *host, const char *service);

int
_set_nonblocking(int fd);

// =============================================================================

struct reactor *
reactor_init(int argc, char *argv[])
{
    struct reactor *server = malloc(sizeof(struct reactor));

    if (server == NULL)
        DIE("(reactor_init) malloc");

    memset(&(server->port), '\0', sizeof(server->port));
    memset(server->port.str, '\0', sizeof(char) * PRTSIZ);
    memset(server->events, '\0', sizeof(struct epoll_event) * MAX_EVENTS);

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
    struct epoll_event ev;

    server->server_fd = _prepare_socket(server->ip, "9999");
    server->epollfd   = epoll_create1(0);

    if (server->epollfd == -1)
        return ERROR;

    ev.events  = EPOLLIN | EPOLLET;
    ev.data.fd = server->server_fd;

    if (epoll_ctl(server->epollfd, EPOLL_CTL_ADD, server->server_fd, &ev) == -1)
        return ERROR;

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
    int fd, nfds, n;
    bool end_of_msg = false;
    ssize_t nread, nsent;
    size_t total_read, total_sent, content_length;
    char buf[BUFSIZ], msg[BUFSIZ];
    struct epoll_event ev;

    for (;;)
    {
        nfds = epoll_wait(server->epollfd, server->events, MAX_EVENTS, -1);

        if (nfds == -1)
            DIE("(reactor_run) epoll_wait");

        for (n = 0; n < nfds; ++n)
        {

            if (server->events[n].data.fd == server->server_fd)
            {

                fd = accept(server->server_fd, NULL, NULL);

                if (fd == -1)
                    DIE("(reactor_run) accept");

                debug("Accepted connection on fd %d\n", fd);

                _set_nonblocking(fd);
                ev.data.ptr = malloc(sizeof(struct reactor_event));
                struct reactor_event *rev =
                    (struct reactor_event *)(ev.data.ptr);

                rev->fd   = fd;
                rev->raw  = malloc(BUFSIZ);
                rev->body = NULL;
                rev->len  = 0;
                rev->req  = (struct request *)malloc(sizeof(struct request));
                rev->res  = (struct response *)malloc(sizeof(struct response));

                ev.events = EPOLLIN | EPOLLET;

                if (epoll_ctl(server->epollfd, EPOLL_CTL_ADD, fd, &ev) == -1)
                    DIE("(reactor_run) epoll_ctl");

                continue;
            }

            struct epoll_event *evp   = &(server->events[n]);
            struct reactor_event *rev = (struct reactor_event *)(evp->data.ptr);

            if (evp->events & (EPOLLERR | EPOLLHUP))
            {
                free(rev->raw);
                free(rev->req);
                free(rev->res);
                close(rev->fd);
                epoll_ctl(server->epollfd, EPOLL_CTL_DEL, rev->fd, NULL);
                free(server->events[n].data.ptr);
            }
            else if (evp->events & EPOLLIN)
            {
                total_read = 0;
                nread      = 0;
                memset(buf, '\0', BUFSIZ);

                for (;;)
                {
                    nread = read_line(rev->fd, rev->raw + rev->len, BUFSIZ, 0);

                    if (errno == EAGAIN)
                        goto wait_to_read;

                    if (nread == -1)
                        DIE("(reactor_run) recv");

                    rev->len += (uint32_t)nread;
                    ((char *)rev->raw)[rev->len] = '\n';
                    rev->len += 1;

                    if (nread == 0)
                        break;
                }

                ev.events = EPOLLOUT | EPOLLET;
                epoll_ctl(server->epollfd, EPOLL_CTL_MOD, rev->fd, &ev);

            wait_to_read:
                continue;
            }
            else if (evp->events & EPOLLOUT)
            {
                nsent      = 0;
                total_sent = 0;
                content_length =
                    (size_t)snprintf(msg, BUFSIZ,
                                     "HTTP/1.1 200 OK\r\n"
                                     "Content-Type: text/html\r\n"
                                     "Content-Length: %ld\r\n"
                                     "Connection: close\r\n"
                                     "Server: reactor/%s\r\n"
                                     "\r\n"
                                     "<html><body>%s</body></html>\r\n",
                                     rev->len, REACTOR_VERSION, rev->raw);

                for (; total_sent < content_length;)
                {
                    nsent = send(rev->fd, msg + total_sent,
                                 content_length - total_sent, MSG_DONTWAIT);

                    if (errno == EAGAIN)
                        goto wait_to_send;

                    if (nsent == -1)
                        DIE("(reactor_run) send");

                    total_sent += (size_t)nsent;
                }

                free(rev->raw);
                free(rev->req);
                free(rev->res);
                close(rev->fd);
                epoll_ctl(server->epollfd, EPOLL_CTL_DEL, rev->fd, NULL);
                free(server->events[n].data.ptr);

            wait_to_send:
                continue;
            }
        }
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
