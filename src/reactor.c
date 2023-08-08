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
                server->events[n].data.ptr =
                    revent_new(server->epollfd, EVENT_SOCKET);

                rev = (struct reactor_event *)server->events[n].data.ptr;

                __reactor_accept(server, rev);
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
                rev = (struct reactor_event *)(server->events[n].data.ptr);
                printf("epoll err: ");

                if (rev->flag == EVENT_TIMER)
                {
                    printf("timer\n");
                    continue;
                }

                printf("socket\n");

                printf("refcnt: %ld\n", rev->__refcnt);
                if (rev->__refcnt > 0)
                    continue;

                ret = revent_destroy(rev);
                if (ret == ERROR)
                    DIE("(reactor_run) revent_destroy");

                ret = revent_destroy(rev->data.rsk->rev_timer);
                if (ret == ERROR)
                    DIE("(reactor_run) revent_destroy");

                server->events[n].data.ptr = NULL;

                continue;
            }

            else if (server->events[n].events & EPOLLRDHUP)
            {
                rev = (struct reactor_event *)(server->events[n].data.ptr);
                printf("epoll err: ");

                if (rev->flag == EVENT_TIMER)
                {
                    printf("timer\n");
                    continue;
                }

                printf("socket\n");

                ret = revent_destroy(rev);
                if (ret == ERROR)
                    DIE("(reactor_run) revent_destroy");

                server->events[n].data.ptr = NULL;

                continue;
            }
            // Some sockets have some data and are ready to read
            else if (server->events[n].events & EPOLLIN)
            {
                printf("epoll in: ");
                rev = (struct reactor_event *)(server->events[n].data.ptr);

                if (rev->flag == EVENT_TIMER)
                {
                    printf("timer\n");
                    _handle_timer(rev);
                    rev->data.rtm = NULL;
                    continue;
                }

                rsk = rev->data.rsk;
                printf("socket: %p\n", rsk);

                if (rsk->req == NULL)
                    rsk->req = request_new();

                if (rsk->req == NULL)
                    DIE("(reactor_run) request_new");

                http_status = request_parse(rsk->req, rsk->fd);

                // Don't continue if the request processing is not ready
                if (http_status == HTTP_READ_AGAIN)
                    goto wait_to_read;

                if (http_status == HTTP_ERROR)
                    DIE("(reactor_run) http_request");

                // Disarm the timer if the request is parsed successfully
                struct reactor_timer *rtm = rsk->rev_timer->data.rtm;
                debug("timer: %d\n", rtm->fd);

                if (rtimer_mod(rtm, 0) == ERROR)
                    DIE("(reactor_run) rtimer_mod");

                debug("Disarmed timer on socket %p\n", rsk);

                // Start to put the task to the queue for thread pool
                if (sem_wait(&(server->pool->empty)) == ERROR)
                    DIE("(reactor_run) sem_wait");
                if (pthread_mutex_lock(&(server->pool->lock)) == ERROR)
                    DIE("(reactor_run) pthread_mutex_lock");

                if (rbuffer_append(server->pool->buffer, rev) == ERROR)
                    DIE("(reactor_run) rbuffer_push");

                if (pthread_mutex_unlock(&(server->pool->lock)) == ERROR)
                    DIE("(reactor_run) pthread_mutex_unlock");
                if (sem_post(&(server->pool->full)) == ERROR)
                    DIE("(reactor_run) sem_post");

                rev->__refcnt++;

                // __reactor_in(server, rev);

            wait_to_read:
                continue;
            }

            // Some sockets wants to send data out
            else if (server->events[n].events & EPOLLOUT)
            {
                debug("epoll out: ");
                int status = 0;
                rev = (struct reactor_event *)(server->events[n].data.ptr);

                if (rev->flag == EVENT_TIMER)
                {
                    debug("timer\n");
                    continue;
                }

                debug("socket\n");

                rsk = rev->data.rsk;
                printf("rsk: %p\n", rsk);

                nsent      = 0;
                total_sent = 0;

                if (rsk->res->body_len > BUFSIZ)
                {
                    rsk->res->__chunked_state = 0;
                    status = response_send_chunked(rsk->res, rsk->fd);

                    if (status == EAGAIN)
                        goto wait_to_send;

                    if (status == EPIPE)
                        goto destroy_reactor_socket;

                    goto destroy_reactor_socket;
                }

                content_length = (size_t)snprintf(
                    msg, BUFSIZ,
                    "HTTP/1.1 %d %s\r\n"
                    "Content-Type: %s\r\n"
                    "Content-Length: %ld\r\n"
                    "Connection: %s\r\n"
                    "Server: reactor/%s\r\n"
                    "\r\n"
                    "%s",
                    rsk->res->status, GET_HTTP_MSG(rsk->res->status),
                    GET_HTTP_CONTENT_TYPE(rsk->res->content_type),
                    rsk->res->body_len,
                    rsk->res->status == 200 ? "keep-alive" : "close",
                    REACTOR_VERSION, rsk->res->body);

                for (; total_sent < content_length;)
                {
                    nsent = send(rsk->fd, msg + total_sent,
                                 content_length - total_sent, MSG_DONTWAIT);

                    if (nsent == -1 && errno == EAGAIN)
                        goto wait_to_send;

                    if (nsent == -1)
                        DIE("(reactor_run) send");

                    total_sent += (size_t)nsent;
                }

            destroy_reactor_socket:
                rev->__refcnt--;

                if (rev->__refcnt > 0)
                    revent_mod(rev, EPOLLIN);
                else
                {
                    ret = revent_destroy(rev);
                    if (ret == ERROR)
                        DIE("(reactor_run) revent_destroy");
                }

            wait_to_send:
                continue;
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
