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

int
main(int argc, const char *argv[])
{
    NOOP(argc);
    NOOP(argv);

    int ret, n;
    struct sockaddr_storage client;
    socklen_t client_len;

    ret = RX_OK;
    rx_core_init(argc, argv);   /* Load config from CLI */
    rx_core_gai();              /* Get address information to load socket */
    rx_core_gni();              /* Get name information to represent socket */
    rx_core_set_nonblocking();  /* Set socket to non-blocking mode */
    rx_core_epoll_create();     /* Create epoll instance */
    rx_core_load_view();        /* Load view engine */
    rx_core_load_ring_buffer(); /* Load ring buffer */
    rx_core_load_thread_pool(); /* Load thread pool */
    rx_core_boot();             /* Make the server listen to connections */

    rx_log(
        LOG_LEVEL_0, LOG_TYPE_INFO,
        "Server has been created."
        "\n\n\tListening on %s:%s\n\n",
        host, service
    );

    /* Main event loop */
    for (;;)
    {
        /*
           Get a list of file descriptors with events that need to be processed.

           Otherwise, epoll_wait() will block indefinitely until an event.
         */
        n = epoll_wait(epoll_fd, events, RX_MAX_EVENTS, -1);
        if (n == -1)
        {
            sprintf(msg, "epoll_wait: %s\n", strerror(errno));
            goto err_epoll;
        }

        for (i = 0; i < n; ++i)
        {
            /*
               If the event comes from the server file descriptor, there is a
               new client that wants to establish a connection with the
               server.

               The remaining events are from client file descriptors.
             */

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

                /* Set client file descriptor to non blocking mode */

                ret = fcntl(client_fd, F_SETFL, O_NONBLOCK);
                if (ret == -1)
                {
                    sprintf(msg, "fcntl: %s\n", strerror(errno));
                    goto err_loop;
                }

                struct rx_connection *conn =
                    malloc(sizeof(struct rx_connection));

                if (conn == NULL)
                {
                    sprintf(msg, "malloc: %s\n", strerror(errno));
                    goto err_loop;
                }

                rx_connection_init(
                    conn, epoll_fd, client_fd, *((struct sockaddr *)&client),
                    client_len
                );

                /*
                   Each epoll_event struct allows client file descriptors to
                   store a pointer. Therefore we can store a pointer to the
                   connection.
                 */

                ev.events   = EPOLLIN | EPOLLET;
                ev.data.ptr = conn;

                ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);
                if (ret != 0)
                {
                    rx_connection_free(conn);
                    free(conn);
                    ev.data.ptr = NULL;

                    sprintf(msg, "epoll_ctl: %s\n", strerror(errno));
                    goto err_loop;
                }

                rx_log(
                    LOG_LEVEL_0, LOG_TYPE_INFO, "New connection from %s:%s\n",
                    conn->host, conn->port
                );

                continue;
            }

            /*
               If the event is an EPOLLIN event, the client has sent data
               (request), and the server needs to read it.
             */

            else if (events[i].events & EPOLLIN)
            {
                struct rx_connection *conn = events[i].data.ptr;
                int fd                     = conn->fd;
                char buf[RX_BUF_SIZE], *end_of_header_ptr;
                ssize_t nread;

                memset(buf, 0, sizeof(buf));

                if (conn->request == NULL)
                {
                    conn->request = calloc(1, sizeof(*conn->request));
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

                if (conn->state == RX_CONNECTION_STATE_READING_HEADER)
                {
                    goto continue_reading;
                }

                // At this moment, the server only supports one request per
                // connection. If the client wishes to make parallele requests,
                // it needs to open connections to the server.
                if (conn->task_num > 0)
                {
                    rx_log(
                        LOG_LEVEL_0, LOG_TYPE_WARN,
                        "Connection on fd %d is busy\n", fd
                    );
                    continue;
                }

                conn->state = RX_CONNECTION_STATE_READING_HEADER;
                conn->task_num++;

                rx_log(
                    LOG_LEVEL_0, LOG_TYPE_INFO, "No. tasks from fd %d: %ld\n",
                    fd, conn->task_num
                );

            continue_reading:
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
                    rx_log(
                        LOG_LEVEL_0, LOG_TYPE_ERROR,
                        "Connection closed by peer\n"
                    );

                    events[i].data.ptr = conn;
                    events[i].events   = EPOLLRDHUP | EPOLLET;
                    ret = epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &events[i]);

                    if (ret == -1)
                    {
                        sprintf(
                            msg, "epoll_ctl (at %s:%d): %s\n", __FILE__,
                            __LINE__, strerror(errno)
                        );
                        goto err_loop;
                    }

                    rx_log(
                        LOG_LEVEL_0, LOG_TYPE_WARN,
                        "\tSocket %d is scheduled to be terminated\n", conn->fd
                    );

                    continue;
                }

                buf[nread] = '\0';

                rx_log(
                    LOG_LEVEL_0, LOG_TYPE_DEBUG,
                    "Received %ld bytes from fd %d\n", nread, fd
                );

                // Check for buffer overflow while reading headers
                size_t hdr_bufsize = conn->header_end - conn->buffer_start;

                switch (conn->state)
                {
                case RX_CONNECTION_STATE_READY:
                    break;
                case RX_CONNECTION_STATE_READING_HEADER:

                    if (hdr_bufsize + nread >= RX_HEADER_BUFFER_SIZE)
                    {
                        rx_log(
                            LOG_LEVEL_0, LOG_TYPE_ERROR,
                            "Header buffer overflow\n"
                        );
                        conn->state = RX_CONNECTION_STATE_CLOSING;
                        break;
                    }

                    // Copy data to header buffer
                    memcpy(conn->buffer_end, buf, nread);

                    conn->buffer_end  = conn->buffer_end + nread;
                    end_of_header_ptr = strstr(conn->buffer_start, "\r\n\r\n");

                    if (end_of_header_ptr == NULL)
                    {
                        rx_log(
                            LOG_LEVEL_0, LOG_TYPE_DEBUG,
                            "No end of header found\n\t%s", conn->buffer_start
                        );
                        continue;
                    }

                    conn->header_end = end_of_header_ptr;
                    conn->body_start = end_of_header_ptr + 4;

                    conn->state          = RX_CONNECTION_STATE_SERVING_REQUEST;
                    conn->request->state = RX_REQUEST_STATE_METHOD;

                    struct rx_task *task = malloc(sizeof(struct rx_task));

                    if (task == NULL)
                    {
                        sprintf(msg, "malloc: %s\n", strerror(errno));
                        goto err_loop;
                    }

                    rx_log(
                        LOG_LEVEL_2, LOG_TYPE_DEBUG, "Header Length: %ld\n",
                        conn->header_end - conn->buffer_start
                    );
                    rx_log(
                        LOG_LEVEL_2, LOG_TYPE_DEBUG, "Body Length: %ld\n",
                        conn->buffer_end - conn->body_start
                    );

                    task->arg    = conn;
                    task->handle = (void *(*)(void *))rx_connection_process;
                    rx_thread_pool_submit(&rx_tp, task);

                    break;

                case RX_CONNECTION_STATE_READING_BODY:
                    // TODO: Implement

                    conn->state = RX_CONNECTION_STATE_SERVING_REQUEST;

                    break;

                default:
                    break;
                }
            }

            /*
               If the evenet is an EPOLLOUT event, the server has finished the
               processing of the request and needs to send the response to the
               client.
             */

            else if (events[i].events & EPOLLOUT)
            {
                struct rx_connection *conn = events[i].data.ptr;
                int fd                     = conn->fd;
                struct rx_response *res    = conn->response;

                ssize_t nsend;

                if (conn->state == RX_CONNECTION_STATE_CLOSING)
                {
                    rx_log(
                        LOG_LEVEL_0, LOG_TYPE_WARN,
                        "[EPOLLOUT] Closing connection on fd %d\n "
                        "detected\n",
                        conn->fd
                    );

                    goto close_connection;
                }

                if (conn->state != RX_CONNECTION_STATE_WRITING_RESPONSE)
                {
                    continue;
                }

                for (nsend = 0; res->resp_buf_offset < res->resp_buf_size;)
                {
                    nsend = send(
                        fd, res->resp_buf + res->resp_buf_offset,
                        res->resp_buf_size - res->resp_buf_offset, MSG_NOSIGNAL
                    );

                    if (nsend == -1)
                    {
                        if (errno == EAGAIN || errno == EWOULDBLOCK)
                        {
                            continue;
                        }
                        else
                        {
                            conn->task_num--;
                            sprintf(
                                msg, "send (at %s:%d): %s\n", __FILE__,
                                __LINE__, strerror(errno)
                            );
                            goto err_loop;
                        }
                    }

                    res->resp_buf_offset += (size_t)nsend;
                }

                rx_log(
                    LOG_LEVEL_0, LOG_TYPE_DEBUG, "Sent %ld bytes to fd %d\n",
                    nsend, fd
                );

                conn->task_num--;
                if (conn->task_num > 0)
                {
                    rx_connection_cleanup(conn);

                    continue;
                }

            close_connection:
                ret = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);

                if (ret == -1)
                {
                    sprintf(
                        msg, "epoll_ctl (at %s:%d): %s\n", __FILE__, __LINE__,
                        strerror(errno)
                    );
                    goto err_loop;
                }

                rx_connection_free(conn);
                free(conn);
                events[i].data.ptr = NULL;

                rx_log(
                    LOG_LEVEL_0, LOG_TYPE_INFO, "Connection closed on fd %d\n",
                    fd
                );
            }

            /*
               If the event is an EPOLLERR or EPOLLRDHUP event, there might be
               an error from the client. Mostly, the client has prematurely
               closed the connection.
             */

            else if (events[i].events & (EPOLLERR | EPOLLRDHUP))
            {
                struct rx_connection *conn = events[i].data.ptr;
                int fd                     = conn->fd;

                conn->state = RX_CONNECTION_STATE_CLOSING;
                conn->task_num--;

                if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL) == -1)
                {
                    sprintf(msg, "epoll_ctl: %s\n", strerror(errno));
                    goto err_loop;
                }

                if (conn->task_num == 0)
                {
                    rx_connection_free(conn);
                    free(conn);
                    events[i].data.ptr = NULL;

                    rx_log(
                        LOG_LEVEL_0, LOG_TYPE_WARN,
                        "Connection closed on fd %d with error %s (event "
                        "code = %ld)\n",
                        fd,
                        events[i].events & EPOLLERR ? "EPOLLERR" : "EPOLLRDHUP",
                        events[i].events
                    );
                }

                continue;
            }
        }
    }

    goto exit_with_grace;

err_loop:
    for (i = 0; i < n; ++i)
    {
        if (close(events[i].data.fd) == -1)
        {
            printf("Failed\n");
            rx_log(LOG_LEVEL_0, LOG_TYPE_ERROR, msg);
            sprintf(
                msg, "close(event.fd=%d): %s\n", events[i].data.fd,
                strerror(errno)
            );
            goto err_epoll;
        }

        if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL) == -1)
        {
            printf("Failed\n");
            rx_log(LOG_LEVEL_0, LOG_TYPE_ERROR, msg);

            rx_log_warn(
                LOG_LEVEL_0, LOG_TYPE_WARN, "Closed connection on fd %d\n",
                events[i].data.fd
            );
        }
    }

err_epoll:
    assert(close(epoll_fd) == 0);
    assert(close(server_fd) == 0);
    rx_view_destroy();

exit_with_grace:
    return ret;
}
