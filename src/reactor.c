#include "reactor.h"

// clang-format off

int _prepare_socket(char *host, const char *service);
int _set_nonblocking(int fd);

// clang-format on

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

    // server->rbuffer = rbuffer_new(8);
    // if (server->rbuffer == NULL)
    // return ERROR;

    server->epollfd = epoll_create1(0);
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

int
reactor_run(struct reactor *server)
{
    int fd, nfds, n, http_status;
    bool end_of_msg = false;
    ssize_t nread, nsent;
    size_t total_read, total_sent, content_length;
    char buf[BUFSIZ], msg[BUFSIZ];
    struct epoll_event ev, *evp;
    struct reactor_event *rev;

    for (;;)
    {
        nfds = epoll_wait(server->epollfd, server->events, MAX_EVENTS, -1);

        if (nfds == -1)
            return ERROR;

        for (n = 0; n < nfds; ++n)
        {
            // New connections
            if (server->events[n].data.fd == server->server_fd)
            {

                fd = accept(server->server_fd, NULL, NULL);

                if (fd == -1)
                    return ERROR;

                debug("Accepted connection on fd %d\n", fd);

                if (_set_nonblocking(fd) == ERROR)
                    return ERROR;

                ev.data.ptr = revent_new(server->epollfd, fd);

                if (ev.data.ptr == NULL)
                    DIE("(reactor_run) revent_new");

                if (revent_add((struct reactor_event *)ev.data.ptr) == ERROR)
                    DIE("(reactor_run) revent_add");

                continue;
            }

            evp = &(server->events[n]);
            rev = (struct reactor_event *)(evp->data.ptr);

            if (evp->events & (EPOLLERR | EPOLLHUP))
            {
                if (revent_destroy(rev) == ERROR)
                    return ERROR;

                evp->data.ptr = NULL;
            }
            // Some sockets have some data and are ready to read
            else if (evp->events & EPOLLIN)
            {
                rev->req = request_new();

                if (rev->req == NULL)
                    DIE("(reactor_run) request_new");

                http_status = request_parse(rev->req, rev->fd);

                if (http_status == HTTP_READ_AGAIN)
                    goto wait_to_read;

                if (http_status == HTTP_ERROR)
                    DIE("(reactor_run) http_request");

                ev.events = EPOLLOUT | EPOLLET;
                if (epoll_ctl(rev->epoll_fd, EPOLL_CTL_MOD, rev->fd, &ev) == -1)
                    DIE("(reactor_run) epoll_ctl");

            wait_to_read:
                continue;
            }

            // Some sockets wants to send data out
            else if (evp->events & EPOLLOUT)
            {
                content_length = snprintf(buf, BUFSIZ,
                                          "<!DOCTYPE>\r\n"
                                          "<html>\r\n"
                                          "<head>\r\n"
                                          "<title>Reactor</title>\r\n"
                                          "</head>\r\n"
                                          "<body>\r\n"
                                          "<h1>Method: %s</h1>\r\n"
                                          "<h1>Path: %s</h1>\r\n"
                                          "<h1>Version: %s</h1>\r\n",
                                          GET_HTTP_METHOD(rev->req->method),
                                          rev->req->path, rev->req->version);

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
                                     "%s",
                                     content_length, REACTOR_VERSION, buf);

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

                if (revent_destroy(rev) == ERROR)
                    return ERROR;

                evp->data.ptr = NULL;

            wait_to_send:
                continue;
            }
        }
    }

    // Not intended to reach here
    return SUCCESS;
}

void
reactor_destroy(struct reactor *server)
{
    if (server->server_fd != -1)
        close(server->server_fd);

    if (server->epollfd != -1)
        close(server->epollfd);

    if (server->rbuffer != NULL)
        rbuffer_free(server->rbuffer);

    free(server);
}

// =============================================================================
