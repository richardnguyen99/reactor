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

struct rx_file template, client_template, server_template;

char *template_ptr, *client_template_ptr, *server_template_ptr;

void *
rx_route_index_get(struct rx_request *req, struct rx_response *res);

void *
rx_route_about_get(struct rx_request *req, struct rx_response *res);

void *
rx_route_static(struct rx_request *req, struct rx_response *res);

void *
rx_request_process(struct rx_connection *conn);

const struct rx_route router_table[] = {{.endpoint = "/",
                                         .resource = "pages/index.html",
                                         .handler  = {.get  = rx_route_index_get,
                                                      .post = NULL,
                                                      .put  = NULL,
                                                      .patch  = NULL,
                                                      .delete = NULL,
                                                      .head   = NULL}},
                                        {.endpoint = "/about",
                                         .resource = "pages/about.html",
                                         .handler  = {.get  = rx_route_about_get,
                                                      .post = NULL,
                                                      .put  = NULL,
                                                      .patch = NULL,
                                                      .head  = NULL}},
                                        {
                                            .endpoint = NULL,
                                            .resource = NULL,
                                            .handler  = {.get    = NULL,
                                                         .post   = NULL,
                                                         .put    = NULL,
                                                         .patch  = NULL,
                                                         .delete = NULL,
                                                         .head   = NULL},
                                        }};

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

    rx_log(LOG_LEVEL_0, LOG_TYPE_INFO, "Open template file... ");

    ret = rx_file_open(&template, "pages/_template.html", O_RDONLY);

    if (ret == RX_ERROR)
    {
        sprintf(msg, "rx_file_open: %s\n", strerror(errno));
        goto err_epoll;
    }

    printf("OK\n");

    rx_log(LOG_LEVEL_0, LOG_TYPE_INFO, "Read template file... ");

    template_ptr =
        mmap(NULL, template.size, PROT_READ, MAP_PRIVATE, template.fd, 0);

    if (template_ptr == MAP_FAILED)
    {
        sprintf(msg, "mmap: %s\n", strerror(errno));
        goto err_epoll;
    }

    printf("OK\n");

    rx_log(LOG_LEVEL_0, LOG_TYPE_INFO, "Open 4xx template file... ");

    ret = rx_file_open(&client_template, "pages/_4xx.html", O_RDONLY);

    if (ret == RX_ERROR)
    {
        sprintf(msg, "rx_file_open: %s\n", strerror(errno));
        goto err_epoll;
    }

    printf("OK\n");

    rx_log(LOG_LEVEL_0, LOG_TYPE_INFO, "Read 4xx template file... ");

    client_template_ptr = mmap(NULL, client_template.size, PROT_READ,
                               MAP_PRIVATE, client_template.fd, 0);

    if (client_template_ptr == MAP_FAILED)
    {
        sprintf(msg, "mmap: %s\n", strerror(errno));
        goto err_epoll;
    }

    printf("OK\n");

    rx_log(LOG_LEVEL_0, LOG_TYPE_INFO, "Open 5xx template file... ");

    ret = rx_file_open(&server_template, "pages/_5xx.html", O_RDONLY);

    if (ret == RX_ERROR)
    {
        sprintf(msg, "rx_file_open: %s\n", strerror(errno));
        goto err_epoll;
    }

    printf("OK\n");

    rx_log(LOG_LEVEL_0, LOG_TYPE_INFO, "Read 5xx template file... ");

    server_template_ptr = mmap(NULL, server_template.size, PROT_READ,
                               MAP_PRIVATE, server_template.fd, 0);

    if (server_template_ptr == MAP_FAILED)
    {
        sprintf(msg, "mmap: %s\n", strerror(errno));
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

                conn->task_num++;

                rx_log(LOG_LEVEL_0, LOG_TYPE_INFO,
                       "No. tasks from fd %d: %ld\n", fd, conn->task_num);

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

                    if (task == NULL)
                    {
                        sprintf(msg, "malloc: %s\n", strerror(errno));
                        goto err_loop;
                    }

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
                                       "Content-Type: %s\r\n"
                                       "Content-Length: %zu\r\n"
                                       "Date: %s\r\n"
                                       "Connection: close\r\n"
                                       "\r\n";

                int fd                  = conn->fd;
                struct rx_response *res = conn->response;

                // HTTP Date format
                const char *date_format = "%a, %d %b %Y %H:%M:%S %Z";
                char date_buf[128];
                time_t now;
                struct tm *tm;

                char *buf;
                ssize_t buf_len, nsend;

                now = time(NULL);
                tm  = gmtime(&now);
                strftime(date_buf, sizeof(date_buf), date_format, tm);

                buf_len =
                    asprintf(&buf, template, host, service, res->content_type,
                             res->content_length, date_buf, res->content);

                if (buf_len == -1)
                {
                    sprintf(msg, "asprintf (at %s:%d): %s\n", __FILE__,
                            __LINE__, strerror(errno));
                    goto err_loop;
                }

                buf = realloc(buf, buf_len + res->content_length);

                if (buf == NULL)
                {
                    sprintf(msg, "realloc (at %s:%d): %s\n", __FILE__, __LINE__,
                            strerror(errno));
                    goto err_loop;
                }

                memcpy(buf + buf_len, res->content, res->content_length);
                buf_len += res->content_length;
                buf[buf_len] = '\0';

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

                rx_log(LOG_LEVEL_0, LOG_TYPE_DEBUG,
                       "Sent headers (%ld bytes) to fd %d\n", nsend, fd);

                free(buf);

                rx_log(LOG_LEVEL_0, LOG_TYPE_DEBUG, "Sent %ld bytes to fd %d\n",
                       nsend, fd);

                conn->task_num--;

                if (conn->task_num != 0)
                {
                    continue;
                }

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

                rx_log(LOG_LEVEL_0, LOG_TYPE_INFO,
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
        rx_file_close(&template);
        rx_file_close(&client_template);
        rx_file_close(&server_template);

        munmap(template_ptr, template.size);
        munmap(client_template_ptr, client_template.size);
        munmap(server_template_ptr, server_template.size);

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

void *
rx_request_process(struct rx_connection *conn)
{
    int ret;
    char *endl;
    char *startl;
    size_t i;
    pthread_t tid = pthread_self();
    clock_t start, end;
    struct rx_route route;
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
    i      = 0;
    memset(&route, 0, sizeof(route));

    rx_log(LOG_LEVEL_0, LOG_TYPE_DEBUG, "[Thread %ld]%4.sHeader length: %ld\n",
           tid, "", conn->header_end - conn->buffer_start);

    endl = strstr(startl, "\r\n");

    ret = rx_request_process_start_line(conn->request, startl, endl - startl);
    if (ret != RX_OK)
    {
        rx_log(LOG_LEVEL_0, LOG_TYPE_ERROR,
               "[Thread %ld]%4.sFailed to process request start line\n", tid,
               "");
        return RX_ERROR_PTR;
    }

    i += endl - startl;
    startl = endl + 2;

    endl = strstr(startl, "\r\n");

    ret = rx_request_process_headers(conn->request, startl, endl - startl);

    conn->state = RX_CONNECTION_STATE_WRITING_RESPONSE;

    end = clock();

    rx_log(LOG_LEVEL_0, LOG_TYPE_INFO,
           "[Thread %ld]%4.sRequest processed successfully (took %.4fms)"
           "\n",
           tid, "", (double)(end - start) / CLOCKS_PER_SEC * 1000);

    ret = rx_route_get(conn->request->uri.path, &route);

    if (ret != RX_OK)
    {
        rx_route_4xx(conn->request, conn->response,
                     RX_HTTP_STATUS_CODE_NOT_FOUND);
        goto end;
    }

    if (conn->request->method == RX_REQUEST_METHOD_GET &&
        route.handler.get != NULL)
    {
        route.handler.get(conn->request, conn->response);
    }
    else if (conn->request->method == RX_REQUEST_METHOD_POST &&
             route.handler.post != NULL)
    {
        route.handler.post(conn->request, conn->response);
    }
    else if (conn->request->method == RX_REQUEST_METHOD_PUT &&
             route.handler.put != NULL)
    {
        route.handler.put(conn->request, conn->response);
    }
    else if (conn->request->method == RX_REQUEST_METHOD_DELETE &&
             route.handler.delete != NULL)
    {
        route.handler.delete(conn->request, conn->response);
    }
    else if (conn->request->method == RX_REQUEST_METHOD_HEAD &&
             route.handler.head != NULL)
    {
        route.handler.head(conn->request, conn->response);
    }

end:
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

void *
rx_route_index_get(struct rx_request *req, struct rx_response *res)
{
    NOOP(req);
    NOOP(res);

    int ret, buflen;
    struct rx_file file;
    char *content;

    memset(&file, 0, sizeof(file));
    ret = rx_file_open(&file, "pages/index.html", O_RDONLY);

    if (ret == RX_ERROR)
    {
        rx_log(LOG_LEVEL_0, LOG_TYPE_ERROR, "Failed to open file\n");
        return RX_ERROR_PTR;
    }

    content = mmap(NULL, file.size, PROT_READ, MAP_PRIVATE, file.fd, 0);

    if (res->content == MAP_FAILED)
    {
        rx_log(LOG_LEVEL_0, LOG_TYPE_ERROR, "mmap: %s\n", strerror(errno));
        return RX_ERROR_PTR;
    }

    buflen = asprintf(&res->content, template_ptr, content, file.size);

    if (buflen == -1)
    {
        rx_log(LOG_LEVEL_0, LOG_TYPE_ERROR, "asprintf: %s\n", strerror(errno));
        return RX_ERROR_PTR;
    }

    res->content_length = buflen;
    res->content_type   = file.mime;

    rx_file_close(&file);

    if (munmap(content, file.size) == -1)
    {
        rx_log(LOG_LEVEL_0, LOG_TYPE_ERROR, "munmap: %s\n", strerror(errno));
        return RX_ERROR_PTR;
    }

    return RX_OK_PTR;
}

void *
rx_route_about_get(struct rx_request *req, struct rx_response *res)
{
    NOOP(req);
    NOOP(res);

    int ret, buflen;
    struct rx_file file;
    char *content;

    memset(&file, 0, sizeof(file));
    ret = rx_file_open(&file, "pages/about.html", O_RDONLY);

    if (ret == RX_ERROR)
    {
        rx_log(LOG_LEVEL_0, LOG_TYPE_ERROR, "Failed to open file\n");
        return RX_ERROR_PTR;
    }

    content = mmap(NULL, file.size, PROT_READ, MAP_PRIVATE, file.fd, 0);

    if (res->content == MAP_FAILED)
    {
        rx_log(LOG_LEVEL_0, LOG_TYPE_ERROR, "mmap: %s\n", strerror(errno));
        return RX_ERROR_PTR;
    }

    buflen = asprintf(&res->content, template_ptr, content, file.size);

    if (buflen == -1)
    {
        rx_log(LOG_LEVEL_0, LOG_TYPE_ERROR, "asprintf: %s\n", strerror(errno));
        return RX_ERROR_PTR;
    }

    res->content_length = buflen;
    res->content_type   = file.mime;

    rx_file_close(&file);

    if (munmap(content, file.size) == -1)
    {
        rx_log(LOG_LEVEL_0, LOG_TYPE_ERROR, "munmap: %s\n", strerror(errno));
        return RX_ERROR_PTR;
    }

    return RX_OK_PTR;
}

void *
rx_route_static(struct rx_request *req, struct rx_response *res)
{
    const size_t resource_len = req->uri.path_end - req->uri.path - 1;

    int ret;
    struct rx_file file;
    char resource[resource_len];

    memset(&file, 0, sizeof(file));
    memcpy(resource, req->uri.path + 1, req->uri.path_end - req->uri.path);
    resource[resource_len] = '\0';

    rx_log(LOG_LEVEL_0, LOG_TYPE_INFO, "Static file request: %s\n", resource);

    ret = rx_file_open(&file, resource, O_RDONLY);

    if (ret == RX_FATAL_WITH_ERROR)
    {
        rx_log(LOG_LEVEL_0, LOG_TYPE_ERROR, "Failed to open file\n");
        return RX_ERROR_PTR;
    }

    res->content =
        (char *)mmap(NULL, file.size, PROT_READ, MAP_PRIVATE, file.fd, 0);

    if (res->content == MAP_FAILED)
    {
        rx_log(LOG_LEVEL_0, LOG_TYPE_ERROR, "mmap: %s\n", strerror(errno));
        return RX_ERROR_PTR;
    }

    res->content_length = file.size;
    res->content_type   = file.mime;

    rx_file_close(&file);

    return RX_OK_PTR;
}

void *
rx_route_4xx(struct rx_request *req, struct rx_response *res, int code)
{
#ifdef RX_DEBUG
    rx_log(LOG_LEVEL_0, LOG_TYPE_WARN, "[Thread %ld]%4.sClient error",
           pthread_self(), "");
#endif
    char msg[80], reason[1024], *buf;
    int buflen;

    switch (code)
    {

    case RX_HTTP_STATUS_CODE_NOT_FOUND:
        sprintf(msg, "Not Found");
        sprintf(reason, "The requested resource (%s) could not be found.",
                req->uri.raw_uri);

        break;

    case RX_HTTP_STATUS_CODE_BAD_REQUEST:
    default:
        sprintf(msg, "Bad Request");
        sprintf(reason, "The server could not process this request due to "
                        "malformed request.");
        break;
    }

    buflen = asprintf(&buf, client_template_ptr, code, msg, reason);

    if (buflen == RX_ERROR)
    {
        rx_log(LOG_LEVEL_0, LOG_TYPE_ERROR, "asprintf: %s\n", strerror(errno));
        return RX_ERROR_PTR;
    }

    res->content_length = asprintf(&res->content, template_ptr, buf);

    if (res->content_length == RX_ERROR)
    {
        rx_log(LOG_LEVEL_0, LOG_TYPE_ERROR, "asprintf: %s\n", strerror(errno));
        return RX_ERROR_PTR;
    }

    res->content_type = "text/html";

    free(buf);

    return NULL;
}
