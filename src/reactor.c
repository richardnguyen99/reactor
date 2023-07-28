#include "reactor.h"

// clang-format off

int _prepare_socket(char *host, const char *service);
int _set_nonblocking(int fd);
void *_handle_request(void * arg);

// clang-format on

// =============================================================================

void
reactor_init(struct reactor **server, int argc, char *argv[])
{
    *server = (struct reactor *)malloc(sizeof(struct reactor));

    if (server == NULL)
        DIE("(reactor_init) malloc");

    memset(&((*server)->port), '\0', sizeof((*server)->port));
    memset((*server)->port.str, '\0', sizeof(char) * PRTSIZ);
    memset((*server)->events, '\0', sizeof(struct epoll_event) * MAX_EVENTS);

    memset((*server)->ip, '\0', INET_ADDRSTRLEN);

    (*server)->port.number = 9999;
    (*server)->server_fd   = -1;

    (*server)->pool = pool_new(8, 8, _handle_request);
    if ((*server)->pool == NULL)
        DIE("(reactor_init) pool_new");

    for (size_t i = 0; i < (*server)->pool->size; ++i)
    {
        if (pthread_create(&((*server)->pool->threads[i]), NULL,
                           _handle_request, (*server)->pool) != 0)
            DIE("(reactor_init) pthread_create");
    }

    if (chdir("public") == -1)
        DIE("(reactor_init) chdir");
}

void
reactor_load(struct reactor *server)
{
    int status = SUCCESS;
    struct epoll_event ev;

    server->server_fd = _prepare_socket(server->ip, "9999");

    server->epollfd = epoll_create1(0);
    if (server->epollfd == -1)
        DIE("(reactor_load) epoll_create1");

    ev.events  = EPOLLIN;
    ev.data.fd = server->server_fd;

    if (epoll_ctl(server->epollfd, EPOLL_CTL_ADD, server->server_fd, &ev) == -1)
        DIE("(reactor_load) epoll_ctl");
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
    int fd, nfds, n, http_status, ret;
    bool end_of_msg = false;
    ssize_t nread, nsent;
    size_t total_read, total_sent, content_length;
    char buf[BUFSIZ], msg[BUFSIZ];
    struct epoll_event *evp;
    struct reactor_event *rev;
    struct reactor_event *pev;

    for (;;)
    {
        nfds = epoll_wait(server->epollfd, server->events, MAX_EVENTS, -1);

        if (nfds == -1)
            return ERROR;

        for (n = 0; n < nfds; ++n)
        {
            if (server->events[n].data.fd == server->server_fd)
            {
                fd = accept(server->server_fd, NULL, NULL);

                if (fd == -1)
                    return ERROR;

                if (_set_nonblocking(fd) == ERROR)
                    return ERROR;

                server->events[n].data.ptr = revent_new(server->epollfd, fd);

                if (rev == NULL)
                    DIE("(reactor_run) revent_new");

                ret = revent_add(
                    (struct reactor_event *)(server->events[n].data.ptr));

                if (ret == ERROR)
                    DIE("(reactor_run) revent_add");
            }
            else if (server->events[n].events & EPOLLERR)
            {
                ret = revent_destroy(
                    (struct reactor_event *)server->events[n].data.ptr);

                if (ret == ERROR)
                    return ERROR;

                server->events[n].data.ptr = NULL;

                continue;
            }
            // Some sockets have some data and are ready to read
            else if (server->events[n].events & EPOLLIN)
            {
                rev = (struct reactor_event *)(server->events[n].data.ptr);

                if (rev->req == NULL)
                    rev->req = request_new();

                if (rev->req == NULL)
                    DIE("(reactor_run) request_new");

                http_status = request_parse(rev->req, rev->fd);

                // Don't continue if the request processing is not ready
                if (http_status == HTTP_READ_AGAIN)
                    goto wait_to_read;

                if (http_status == HTTP_ERROR)
                    DIE("(reactor_run) http_request");

                // Start to put the task to the queue for thread pool
                if (sem_wait(&(server->pool->empty)) == ERROR)
                    DIE("(reactor_run) sem_wait");
                if (pthread_mutex_lock(&(server->pool->lock)) == ERROR)
                    return ERROR;

                if (revent_mod(rev, EPOLLHUP) == ERROR)
                    DIE("(reactor_run) revent_mod");

                if (rbuffer_append(server->pool->buffer, rev) == ERROR)
                    DIE("(reactor_run) rbuffer_push");

                if (pthread_mutex_unlock(&(server->pool->lock)) == ERROR)
                    DIE("(reactor_run) pthread_mutex_unlock");
                if (sem_post(&(server->pool->full)) == ERROR)
                    DIE("(reactor_run) sem_post");

            wait_to_read:
                continue;
            }

            // Some sockets wants to send data out
            else if (server->events[n].events & EPOLLOUT)
            {
                rev        = (struct reactor_event *)server->events[n].data.ptr;
                nsent      = 0;
                total_sent = 0;

                content_length = (size_t)snprintf(
                    msg, BUFSIZ,
                    "HTTP/1.1 %d %s\r\n"
                    "Content-Type: %s\r\n"
                    "Content-Length: %ld\r\n"
                    "Connection: close\r\n"
                    "Server: reactor/%s\r\n"
                    "\r\n"
                    "%s",
                    rev->res->status, GET_HTTP_MSG(rev->res->status),
                    GET_HTTP_CONTENT_TYPE(rev->res->content_type),
                    rev->res->body_len, REACTOR_VERSION, rev->res->body);

                for (; total_sent < content_length;)
                {
                    nsent = send(rev->fd, msg + total_sent,
                                 content_length - total_sent, MSG_DONTWAIT);

                    if (nsent == -1 && errno == EAGAIN)
                        goto wait_to_send;

                    if (nsent == -1)
                        DIE("(reactor_run) send");

                    total_sent += (size_t)nsent;
                }

                if (revent_destroy(rev) == ERROR)
                    return ERROR;

                server->events[n].data.ptr = NULL;

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

    if (server->pool != NULL)
        pool_free(server->pool);

    free(server);
}

// =============================================================================
