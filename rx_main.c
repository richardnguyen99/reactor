/* MIT License
 *
 * Copyright (c) 2023 Richard H. Nguyen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <rx_config.h>
#include <rx_core.h>

#define RX_TASK_TYPE_PROCESS_REQUEST  1
#define RX_TASK_TYPE_PROCESS_RESPONSE 2

struct rx_task
{
    void *(*handle)(void *);
    void *arg;

    int task_type;
};

struct rx_ring
{
    size_t size;
    size_t cap;

    size_t in;
    size_t out;

    struct rx_task *tasks[256];
};

struct rx_thread_pool
{
    pthread_t threads[8];
    size_t nthreads;

    pthread_mutex_t lock;
    sem_t empty;
    sem_t full;

    struct rx_ring *ring;
};

void
rx_ring_init(struct rx_ring *ring);

void
rx_ring_push(struct rx_ring *ring, struct rx_task *task);

struct rx_task *
rx_ring_pop(struct rx_ring *ring);

void *
rx_thread_pool_worker(void *arg);

int
rx_thread_pool_init(struct rx_thread_pool *pool, struct rx_ring *ring);

int
rx_thread_pool_submit(struct rx_thread_pool *pool, struct rx_task *task);

void *
rx_request_process(struct rx_connection *conn);

int
main(int argc, const char *argv[])
{
    NOOP(argc);
    NOOP(argv);

    int server_fd, client_fd, ret, epoll_fd, n, i;
    struct addrinfo hints, *res, *p;
    struct sockaddr server;
    struct sockaddr_storage client;
    socklen_t server_len, client_len;
    struct epoll_event ev, events[RX_MAX_EVENTS];
    char msg[1024], host[NI_MAXHOST], service[NI_MAXSERV];

    rx_log(LOG_LEVEL_0, LOG_TYPE_INFO, "Load host address information... ");

    memset(msg, 0, sizeof(msg));
    memset(&hints, 0, sizeof(hints));

    hints.ai_family   = AF_INET;     /* IPv4 */
    hints.ai_socktype = SOCK_STREAM; /* TCP */
    hints.ai_flags    = AI_PASSIVE;  /* Listen on all interfaces */

    if (getaddrinfo(NULL, "8080", &hints, &res) != 0)
    {
        sprintf(msg, "getaddrinfo: %s\n", gai_strerror(errno));
        goto err_addrinfo;
    }

    for (p = res; p != NULL; p = p->ai_next)
    {
        server_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

        if (server_fd == -1)
        {
            perror("socket");
            continue;
        }

        ret = bind(server_fd, p->ai_addr, p->ai_addrlen);
        if (ret == -1)
        {
            if (close(server_fd) == -1)
            {
                sprintf(msg, "close: %s\n", strerror(errno));
                goto err_addrinfo;
            }

            printf("Failed\n");
            rx_log_error("bind: %s. Trying a new one... ", strerror(errno));
            continue;
        }

        break;
    }

    if (p == NULL)
    {
        sprintf(msg, "Failed to bind socket to any address\n");
        goto err_addrinfo;
    }

    memcpy(&server, p->ai_addr, p->ai_addrlen);
    server_len = p->ai_addrlen;

    freeaddrinfo(res);
    printf("OK\n");

    rx_log(LOG_LEVEL_0, LOG_TYPE_INFO, "Get host and service name... ");

    ret = getnameinfo(&server, server_len, host, NI_MAXHOST, service,
                      NI_MAXSERV, NI_NUMERICSERV);

    if (ret != 0)
    {
        sprintf(msg, "getnameinfo: %s\n", gai_strerror(errno));
        goto err_addrinfo;
    }

    printf("OK\n");

    rx_log(LOG_LEVEL_0, LOG_TYPE_INFO, "Set socket to non-blocking mode... ");
    ret = fcntl(server_fd, F_SETFL, O_NONBLOCK);
    if (ret == -1)
    {
        sprintf(msg, "fcntl: %s\n", strerror(errno));
        goto err_socket;
    }
    printf("OK\n");

    rx_log(LOG_LEVEL_0, LOG_TYPE_INFO, "Create epoll instance... ");

    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1)
    {
        sprintf(msg, "epoll_create1: %s\n", strerror(errno));
        goto err_epoll;
    }

    printf("OK\n");

    rx_log(LOG_LEVEL_0, LOG_TYPE_INFO,
           "Add server socket to epoll instance... ");

    ev.events  = EPOLLIN;
    ev.data.fd = server_fd;
    ret        = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev);

    if (ret == -1)
    {
        sprintf(msg, "epoll_ctl: %s\n", strerror(errno));
        goto err_epoll;
    }

    printf("OK\n");

    rx_log(LOG_LEVEL_0, LOG_TYPE_INFO, "Initialize ring buffer... ");

    struct rx_ring *ring = malloc(sizeof(*ring));
    if (ring == NULL)
    {
        sprintf(msg, "malloc: %s\n", strerror(errno));
        goto err_epoll;
    }

    rx_ring_init(ring);

    printf("OK\n");

    rx_log(LOG_LEVEL_1, LOG_TYPE_DEBUG, "Ring buffer address: %p\n", ring);
    rx_log(LOG_LEVEL_1, LOG_TYPE_DEBUG, "Ring buffer size: %ld\n", ring->size);
    rx_log(LOG_LEVEL_1, LOG_TYPE_DEBUG, "Ring buffer capacity: %ld\n",
           ring->cap);

    rx_log(LOG_LEVEL_0, LOG_TYPE_INFO, "Initialize thread pool... ");

    struct rx_thread_pool *pool = malloc(sizeof(*pool));
    if (pool == NULL)
    {
        sprintf(msg, "malloc: %s\n", strerror(errno));
        goto err_epoll;
    }

    rx_thread_pool_init(pool, ring);

    printf("OK\n");

    rx_log(LOG_LEVEL_1, LOG_TYPE_DEBUG, "Thread pool address: %p\n", pool);
    rx_log(LOG_LEVEL_1, LOG_TYPE_DEBUG, "No. active threads: %ld\n",
           pool->nthreads);
    rx_log(LOG_LEVEL_1, LOG_TYPE_DEBUG, "Ring buffer address: %p\n",
           pool->ring);

    rx_log(LOG_LEVEL_0, LOG_TYPE_INFO, "Start listening on server socket... ");

    ret = listen(server_fd, RX_LISTEN_BACKLOG);
    if (ret == -1)
    {
        sprintf(msg, "listen: %s\n", strerror(errno));
        goto err_epoll;
    }

    printf("OK\n");

    rx_log(LOG_LEVEL_0, LOG_TYPE_INFO,
           "Server has been created."
           "\n\n\tListening on %s:%s\n\n",
           host, service);

    for (;;)
    {
        n = epoll_wait(epoll_fd, events, RX_MAX_EVENTS, -1);
        if (n == -1)
        {
            sprintf(msg, "epoll_wait: %s\n", strerror(errno));
            goto err_epoll;
        }

        for (i = 0; i < n; ++i)
        {
            if (events[i].data.fd == server_fd)
            {
                memset(&client, 0, sizeof(client));
                client_len = sizeof(client);
                client_fd =
                    accept(server_fd, (struct sockaddr *)&client, &client_len);

                if (client_fd == -1)
                {
                    if (errno == EAGAIN || errno == EWOULDBLOCK)
                    {
                        continue;
                    }
                    else
                    {
                        sprintf(msg, "accept: %s\n", strerror(errno));
                        goto err_epoll;
                    }
                }

                ret = fcntl(client_fd, F_SETFL, O_NONBLOCK);
                if (ret == -1)
                {
                    sprintf(msg, "fcntl: %s\n", strerror(errno));
                    goto err_loop;
                }

                struct rx_connection *conn = malloc(sizeof(*conn));
                if (conn == NULL)
                {
                    sprintf(msg, "malloc: %s\n", strerror(errno));
                    goto err_loop;
                }

                rx_connection_init(conn, epoll_fd, client_fd,
                                   *((struct sockaddr *)&client), client_len);
                ev.events   = EPOLLIN | EPOLLET;
                ev.data.ptr = conn;

                ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);
                if (ret != 0)
                {
                    sprintf(msg, "epoll_ctl: %s\n", strerror(errno));
                    goto err_loop;
                }

                rx_log(LOG_LEVEL_0, LOG_TYPE_INFO,
                       "New connection from %s:%s\n", conn->host, conn->port);

                continue;
            }
            else if (events[i].events & EPOLLIN)
            {
                struct rx_connection *conn = events[i].data.ptr;
                int fd                     = conn->fd;
                char buf[RX_BUF_SIZE], *end_of_header_ptr;
                ssize_t nread;

                memset(buf, 0, sizeof(buf));

                if (conn->request == NULL)
                {
                    conn->request = malloc(sizeof(*conn->request));
                    if (conn->request == NULL)
                    {
                        sprintf(msg, "malloc: %s\n", strerror(errno));
                        goto err_loop;
                    }

                    (void)rx_request_init(conn->request);
                }

                if (conn->response == NULL)
                {
                    conn->response = calloc(1, sizeof(*conn->response));

                    if (conn->response == NULL)
                    {
                        sprintf(msg, "calloc: %s\n", strerror(errno));
                        goto err_loop;
                    }

                    (void)rx_response_init(conn->response);
                }

                nread = recv(fd, buf, sizeof(buf), 0);

                if (nread == -1)
                {
                    if (errno == EAGAIN || errno == EWOULDBLOCK)
                    {
                        continue;
                    }
                    else
                    {
                        sprintf(msg, "read: %s\n", strerror(errno));
                        goto err_loop;
                    }
                }
                else if (nread == 0)
                {
                    rx_log(LOG_LEVEL_0, LOG_TYPE_ERROR,
                           "Connection closed by peer\n");

                    events[i].data.ptr = conn;
                    events[i].events   = EPOLLRDHUP | EPOLLET;
                    ret = epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &events[i]);

                    if (ret == -1)
                    {
                        sprintf(msg, "epoll_ctl (at %s:%d): %s\n", __FILE__,
                                __LINE__, strerror(errno));
                        goto err_loop;
                    }

                    rx_log(LOG_LEVEL_0, LOG_TYPE_WARN,
                           "\tSocket %d is scheduled to be terminated\n",
                           conn->fd);

                    continue;
                }

                buf[nread]  = '\0';
                conn->state = RX_CONNECTION_STATE_READING_HEADER;

                rx_log(LOG_LEVEL_0, LOG_TYPE_DEBUG,
                       "Received %ld bytes from fd %d\n", nread, fd);

                // Check for buffer overflow while reading headers
                size_t hdr_bufsize = conn->header_end - conn->buffer_start;
                // size_t bufsize     = conn->buffer_end - conn->buffer_start;

                switch (conn->state)
                {
                case RX_CONNECTION_STATE_READY:
                    break;
                case RX_CONNECTION_STATE_READING_HEADER:

                    if (hdr_bufsize + nread >= RX_HEADER_BUFFER_SIZE)
                    {
                        rx_log(LOG_LEVEL_0, LOG_TYPE_ERROR,
                               "Header buffer overflow\n");
                        conn->state = RX_CONNECTION_STATE_CLOSING;
                        break;
                    }

                    // Copy data to header buffer
                    memcpy(conn->buffer_end, buf, nread);

                    conn->buffer_end  = conn->buffer_end + nread;
                    end_of_header_ptr = strstr(conn->buffer_start, "\r\n\r\n");

                    if (end_of_header_ptr == NULL)
                    {
                        rx_log(LOG_LEVEL_0, LOG_TYPE_DEBUG,
                               "No end of header found\n\t%s",
                               conn->buffer_start);
                        continue;
                    }

                    conn->header_end = end_of_header_ptr;
                    conn->body_start = end_of_header_ptr + 4;

                    conn->state          = RX_CONNECTION_STATE_SERVING_REQUEST;
                    conn->request->state = RX_REQUEST_STATE_METHOD;

                    struct rx_task *task = malloc(sizeof(*task));

                    rx_log(LOG_LEVEL_2, LOG_TYPE_DEBUG, "Header Length: %ld\n",
                           conn->header_end - conn->buffer_start);
                    rx_log(LOG_LEVEL_2, LOG_TYPE_DEBUG, "Body Length: %ld\n",
                           conn->buffer_end - conn->body_start);

                    task->arg    = conn;
                    task->handle = (void *(*)(void *))rx_request_process;
                    rx_thread_pool_submit(pool, task);

                    break;

                case RX_CONNECTION_STATE_READING_BODY:
                    // TODO: Implement

                    conn->state = RX_CONNECTION_STATE_SERVING_REQUEST;

                    break;

                default:
                    break;
                }

                events[i].events = EPOLLOUT | EPOLLET;
                ret = epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &events[i]);

                if (ret == -1)
                {
                    sprintf(msg, "epoll_ctl (at %s:%d): %s\n", __FILE__,
                            __LINE__, strerror(errno));
                    goto err_loop;
                }
            }
            else if (events[i].events & EPOLLOUT)
            {
                struct rx_connection *conn = events[i].data.ptr;

                if (conn->state != RX_CONNECTION_STATE_WRITING_RESPONSE)
                {
                    continue;
                }

                const char *template = "HTTP/1.1 200 OK\r\n"
                                       "Host: %s:%s\r\n"
                                       "Content-Type: text/plain\r\n"
                                       "Content-Length: 12\r\n"
                                       "Date: %s\r\n"
                                       "Connection: close\r\n"
                                       "\r\n"
                                       "Hello, World!";

                int fd = conn->fd;

                // HTTP Date format
                const char *date_format = "%a, %d %b %Y %H:%M:%S %Z";
                char date_buf[128];

                time_t now    = time(NULL);
                struct tm *tm = gmtime(&now);

                strftime(date_buf, sizeof(date_buf), date_format, tm);

                char *buf;
                ssize_t buf_len, nsend;

                buf_len = asprintf(&buf, template, host, service, date_buf);

                if (buf_len == -1)
                {
                    sprintf(msg, "asprintf (at %s:%d): %s\n", __FILE__,
                            __LINE__, strerror(errno));
                    goto err_loop;
                }

                for (;;)
                {
                    nsend = send(fd, buf, buf_len, MSG_NOSIGNAL);

                    if (nsend == -1)
                    {
                        if (errno == EAGAIN || errno == EWOULDBLOCK)
                        {
                            continue;
                        }
                        else
                        {
                            sprintf(msg, "send (at %s:%d): %s\n", __FILE__,
                                    __LINE__, strerror(errno));
                            goto err_loop;
                        }
                    }

                    break;
                }

                free(buf);

                rx_log(LOG_LEVEL_1, LOG_TYPE_DEBUG, "Sent %ld bytes to fd %d\n",
                       nsend, fd);

                ret = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);

                if (ret == -1)
                {
                    sprintf(msg, "epoll_ctl (at %s:%d): %s\n", __FILE__,
                            __LINE__, strerror(errno));
                    goto err_loop;
                }

                rx_connection_free(conn);
                free(conn);
                events[i].data.ptr = NULL;

                rx_log(LOG_LEVEL_1, LOG_TYPE_DEBUG,
                       "Connection closed on fd %d\n", fd);
            }
            else if (events[i].events & (EPOLLERR | EPOLLRDHUP))
            {
                struct rx_connection *conn = events[i].data.ptr;
                int fd                     = conn->fd;

                rx_log(LOG_LEVEL_0, LOG_TYPE_WARN, "Closing fd %d\n", fd);

                if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL) == -1)
                {
                    sprintf(msg, "epoll_ctl: %s\n", strerror(errno));
                    goto err_loop;
                }

                rx_connection_free(conn);
                free(conn);
                events[i].data.ptr = NULL;

                rx_log(LOG_LEVEL_0, LOG_TYPE_WARN,
                       "Connection closed on fd %d with error %s (event "
                       "code = %ld)\n",
                       fd,
                       events[i].events & EPOLLERR ? "EPOLLERR" : "EPOLLRDHUP",
                       events[i].events);
            }
        }
    }

    ret = EXIT_SUCCESS;
    goto exit_with_grace;

err_loop:
    for (i = 0; i < n; ++i)
    {
        if (close(events[i].data.fd) == -1)
        {
            printf("Failed\n");
            rx_log(LOG_LEVEL_0, LOG_TYPE_ERROR, msg);
            sprintf(msg, "close(event.fd=%d): %s\n", events[i].data.fd,
                    strerror(errno));
            goto err_epoll;
        }

        if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL) == -1)
        {
            printf("Failed\n");
            rx_log(LOG_LEVEL_0, LOG_TYPE_ERROR, msg);

            rx_log_warn(LOG_LEVEL_0, LOG_TYPE_WARN,
                        "Closed connection on fd %d\n", events[i].data.fd);
        }

    err_epoll:
        close(epoll_fd);

    err_socket:
        close(server_fd);

    err_addrinfo:
        printf("Failed\n");
        rx_log(LOG_LEVEL_0, LOG_TYPE_ERROR, msg);
        ret = EXIT_FAILURE;

    exit_with_grace:
        return ret;
    }
}

void
rx_ring_init(struct rx_ring *ring)
{
    ring->size = 0;
    ring->cap  = 256;
    ring->in   = 0;
    ring->out  = 0;

    for (size_t i = 0; i < ring->cap; i++)
    {
        ring->tasks[i] = NULL;
    }
}

void
rx_ring_push(struct rx_ring *ring, struct rx_task *task)
{
    if (ring->size == ring->cap)
    {
        return;
    }

    ring->tasks[ring->in] = task;
    ring->in              = (ring->in + 1) % ring->cap;
    ring->size++;
}

struct rx_task *
rx_ring_pop(struct rx_ring *ring)
{
    if (ring->size == 0)
    {
        return NULL;
    }

    struct rx_task *task = ring->tasks[ring->out];
    ring->out            = (ring->out + 1) % ring->cap;
    ring->size--;

    return task;
}

void *
rx_thread_pool_worker(void *arg)
{
    struct rx_thread_pool *pool = arg;

    for (;;)
    {
        sem_wait(&pool->full);
        pthread_mutex_lock(&pool->lock);

        struct rx_task *task = rx_ring_pop(pool->ring);

        pthread_mutex_unlock(&pool->lock);
        sem_post(&pool->empty);

        if (task == NULL)
        {
            continue;
        }

        task->handle(task->arg);

        free(task);
    }
}

int
rx_thread_pool_init(struct rx_thread_pool *pool, struct rx_ring *ring)
{
    int ret;

    pool->nthreads = 8;
    pool->ring     = ring;

    ret = pthread_mutex_init(&pool->lock, NULL);

    if (ret == -1)
    {
        rx_log(LOG_LEVEL_0, LOG_TYPE_ERROR, "pthread_mutex_init: %s\n",
               strerror(errno));
        return RX_ERROR;
    }

    ret = sem_init(&pool->empty, 0, 0);

    if (ret == -1)
    {
        rx_log(LOG_LEVEL_0, LOG_TYPE_ERROR, "sem_init: %s\n", strerror(errno));
        return RX_ERROR;
    }

    ret = sem_init(&pool->full, 0, 256);

    if (ret == -1)
    {
        rx_log(LOG_LEVEL_0, LOG_TYPE_ERROR, "sem_init: %s\n", strerror(errno));
        return RX_ERROR;
    }

    for (size_t i = 0; i < 8; i++)
    {
        ret = pthread_create(&pool->threads[i], NULL, rx_thread_pool_worker,
                             pool);

        if (ret != 0)
        {
            rx_log(LOG_LEVEL_0, LOG_TYPE_ERROR, "pthread_create: %s\n",
                   strerror(errno));
            pool->nthreads--;
        }
    }

    return RX_OK;
}

int
rx_thread_pool_submit(struct rx_thread_pool *pool, struct rx_task *task)
{
    int ret;

    ret = sem_wait(&pool->empty);

    if (ret == -1)
    {
        rx_log(LOG_LEVEL_0, LOG_TYPE_ERROR, "sem_wait: %s\n", strerror(errno));
        return RX_ERROR;
    }

    ret = pthread_mutex_lock(&pool->lock);

    if (ret == -1)
    {
        rx_log(LOG_LEVEL_0, LOG_TYPE_ERROR, "pthread_mutex_lock: %s\n",
               strerror(errno));
        return RX_ERROR;
    }

    rx_ring_push(pool->ring, task);

    ret = pthread_mutex_unlock(&pool->lock);

    if (ret == -1)
    {
        rx_log(LOG_LEVEL_0, LOG_TYPE_ERROR, "pthread_mutex_unlock: %s\n",
               strerror(errno));
        return RX_ERROR;
    }

    ret = sem_post(&pool->full);

    if (ret == -1)
    {
        rx_log(LOG_LEVEL_0, LOG_TYPE_ERROR, "sem_post: %s\n", strerror(errno));
        return RX_ERROR;
    }

    rx_log(LOG_LEVEL_0, LOG_TYPE_DEBUG, "Submit task to thread pool\n");

    return RX_OK;
}

void *
rx_request_process(struct rx_connection *conn)
{
    int ret;
    char *endl;
    char *startl;
    size_t hdrsz, i;
    pthread_t tid = pthread_self();
    clock_t start, end;
    struct epoll_event event;

    rx_log(LOG_LEVEL_0, LOG_TYPE_INFO,
           "[Thread %ld]%4.sProcessing request from %s:%s (socket = %d)\n", tid,
           "", conn->host, conn->port, conn->fd);

    if (conn->state != RX_CONNECTION_STATE_SERVING_REQUEST)
    {
        rx_log(LOG_LEVEL_0, LOG_TYPE_ERROR,
               "Connection is not ready to serve request\n");
        return RX_ERROR_PTR;
    }

    start  = clock();
    startl = conn->buffer_start;
    endl   = startl;
    hdrsz  = conn->header_end - conn->buffer_start;
    i      = 0;

    rx_log(LOG_LEVEL_0, LOG_TYPE_DEBUG, "[Thread %ld]%4.sHeader length: %ld\n",
           tid, "", conn->header_end - conn->buffer_start);

    endl = strstr(startl, "\r\n");
    // ret = rx_request_starting_line(conn->request, conn->response, startl,
    // endl);

    if (ret != RX_OK)
    {
    }

    i += endl - startl;
    startl = endl + 2;

    endl = strstr(startl, "\r\n");

    i += endl - startl;
    startl = endl + 2;

    for (; i < hdrsz;)
    {
        endl = strstr(startl, "\r\n");

        rx_log(LOG_LEVEL_0, LOG_TYPE_DEBUG, "[Thread %ld]%4.s%.*s\n", tid, "",
               (int)(endl - startl), startl);

        i += endl - startl;
        startl = endl + 2;
    }

    conn->state = RX_CONNECTION_STATE_WRITING_RESPONSE;

    end = clock();

    rx_log(LOG_LEVEL_0, LOG_TYPE_INFO,
           "[Thread %ld]%4.sRequest processed successfully (took %.4fms)"
           "\n",
           tid, "", (double)(end - start) / CLOCKS_PER_SEC * 1000);

    event.events   = EPOLLOUT | EPOLLET;
    event.data.ptr = conn;

    ret = epoll_ctl(conn->efd, EPOLL_CTL_MOD, conn->fd, &event);

    if (ret == -1)
    {
        rx_log(LOG_LEVEL_0, LOG_TYPE_ERROR, "epoll_ctl: %s\n", strerror(errno));
        return RX_ERROR_PTR;
    }

    rx_log(LOG_LEVEL_0, LOG_TYPE_INFO,
           "[Thread %ld]%4.sSubmit finished task to event poll\n", tid, "");

    return RX_OK_PTR;
}
