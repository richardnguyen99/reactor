#include "reactor.h"

// clang-format off

int _prepare_socket(char *host, const char *service);
int _set_nonblocking(int fd);
void *_handle_request(void * arg);
void _handle_timer(struct reactor_event *rtm);

void __reactor_accept(struct reactor *server, struct reactor_event *rev);

extern inline 
void __reactor_in(struct reactor *server, struct reactor_event *rev);

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

    (*server)->pool = pool_new(16, 64, _handle_request);
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
    int timer_fd, fd, nfds, n, http_status, ret;
    bool end_of_msg = false;
    ssize_t nread, nsent;
    size_t total_read, total_sent, content_length;
    char buf[BUFSIZ], msg[BUFSIZ];
    struct epoll_event *evp;
    struct reactor_event *rev;
    struct reactor_socket *pev;
    struct reactor_socket *rsk;

    for (;;)
    {
        nfds = epoll_wait(server->epollfd, server->events, MAX_EVENTS, -1);

        if (nfds == -1)
            DIE("(reactor_run) epoll_wait");

        for (n = 0; n < nfds; ++n)
        {
            if (server->events[n].data.fd == server->server_fd)
            {

                debug("Accept new connection\n");
                server->events[n].data.ptr =
                    revent_new(server->epollfd, EVENT_SOCKET);

                rev = (struct reactor_event *)(server->events[n].data.ptr);

                __reactor_accept(server, rev);

                debug("New connection from %s:%ld\n\n",
                      inet_ntoa(rev->data.rsk->client.sin_addr),
                      ntohs(rev->data.rsk->client.sin_port));
            }

            else if (((struct reactor_socket *)(server->events[n].data.ptr))
                         ->fd == timer_fd)
            {
                rsocket_destroy(
                    (struct reactor_socket *)server->events[n].data.ptr);

                continue;
            }

            else if (server->events[n].events & (EPOLLERR | EPOLLHUP))
            {
                debug("EPOLLERR | EPOLLHUP\n");
                rev = (struct reactor_event *)(server->events[n].data.ptr);
                if (rev->flag == EVENT_TIMER)
                    continue;

                debug("ref count: %d\n", rev->refcnt);
                if (rev->refcnt > 0)
                {
                    rev->state = 1;
                    continue;
                }

                ret = revent_destroy(rev);
                if (ret == ERROR)
                    DIE("(reactor_run) revent_destroy");

                server->events[n].data.ptr = NULL;

                continue;
            }

            else if (server->events[n].events & EPOLLRDHUP)
            {
                rev = (struct reactor_event *)(server->events[n].data.ptr);

                if (rev->flag == EVENT_TIMER)
                    continue;

                if (rev->refcnt > 0)
                {
                    rev->state = 1;
                    continue;
                }

                ret = revent_destroy(rev);
                if (ret == ERROR)
                    DIE("(reactor_run) revent_destroy");

                server->events[n].data.ptr = NULL;

                continue;
            }

            // Some sockets have some data and are ready to read
            else if (server->events[n].events & EPOLLIN)
            {
                rev = (struct reactor_event *)(server->events[n].data.ptr);

                if (rev->flag == EVENT_TIMER)
                {
                    _handle_timer(rev);
                    continue;
                }

                __reactor_in(server, rev);
            }
            else if (server->events[n].events & EPOLLOUT)
            {
                printf("EPOLLOUT\n");

                rev = (struct reactor_event *)(server->events[n].data.ptr);

                if (rev->flag == EVENT_TIMER)
                    continue;

                debug("ref count: %d\n", rev->refcnt);
                if (rev->refcnt == 0)
                    revent_destroy(rev);
            }
        }
    }

    // Not intended to reach here
    return ERROR;
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
