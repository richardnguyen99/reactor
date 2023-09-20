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

// ANSI escape codes for text color
#define ANSI_COLOR_RED   "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_RESET "\x1b[0m"

void
debug_printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    printf(ANSI_COLOR_GREEN "%-8s" ANSI_COLOR_RESET, "[INFO]");
    vprintf(fmt, args);
    va_end(args);
}

void
error_printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    printf(ANSI_COLOR_RED "[ERROR] " ANSI_COLOR_RESET);
    vprintf(fmt, args);
    va_end(args);
}

int
main(int argc, const char *argv[])
{
    int server_fd, client_fd, ret, epoll_fd, n, i;
    struct addrinfo hints, *res, *p;
    struct sockaddr server;
    struct sockaddr_storage client;
    socklen_t server_len, client_len;
    struct epoll_event ev, events[RX_MAX_EVENTS];
    char msg[1024], host[NI_MAXHOST], service[NI_MAXSERV];

    debug_printf("Load host address information... ");

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

            error_printf("\nbind: %s\n", strerror(errno));
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

    debug_printf("Get host and service name... ");

    ret = getnameinfo(&server, server_len, host, NI_MAXHOST, service,
                      NI_MAXSERV, NI_NUMERICSERV);

    if (ret != 0)
    {
        sprintf(msg, "getnameinfo: %s\n", gai_strerror(errno));
        goto err_addrinfo;
    }

    printf("OK\n");

    debug_printf("Set socket to non-blocking mode... ");
    ret = fcntl(server_fd, F_SETFL, O_NONBLOCK);
    if (ret == -1)
    {
        sprintf(msg, "fcntl: %s\n", strerror(errno));
        goto err_socket;
    }
    printf("OK\n");

    debug_printf("Create epoll instance... ");

    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1)
    {
        sprintf(msg, "epoll_create1: %s\n", strerror(errno));
        goto err_epoll;
    }

    printf("OK\n");

    debug_printf("Add server socket to epoll instance... ");

    ev.events  = EPOLLIN;
    ev.data.fd = server_fd;
    ret        = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev);

    if (ret == -1)
    {
        sprintf(msg, "epoll_ctl: %s\n", strerror(errno));
        goto err_epoll;
    }

    printf("OK\n");

    debug_printf("Start listening on server socket... ");

    ret = listen(server_fd, RX_LISTEN_BACKLOG);
    if (ret == -1)
    {
        sprintf(msg, "listen: %s\n", strerror(errno));
        goto err_epoll;
    }

    printf("OK\n");

    debug_printf("Server has been created."
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

                char client_host[NI_MAXHOST], client_service[NI_MAXSERV];

                ret = getnameinfo((struct sockaddr *)&client, client_len,
                                  client_host, NI_MAXHOST, client_service,
                                  NI_MAXSERV, NI_NUMERICSERV);

                if (ret != 0)
                {
                    sprintf(msg, "getnameinfo: %s (error code = %d)\n",
                            gai_strerror(ret), ret);
                    goto err_loop;
                }

                ret = fcntl(client_fd, F_SETFL, O_NONBLOCK);
                if (ret == -1)
                {
                    sprintf(msg, "fcntl: %s\n", strerror(errno));
                    goto err_loop;
                }

                ev.events  = EPOLLIN | EPOLLET;
                ev.data.fd = client_fd;

                ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);
                if (ret != 0)
                {
                    sprintf(msg, "epoll_ctl: %s\n", strerror(errno));
                    goto err_loop;
                }

                debug_printf("New connection from %s:%s\n", client_host,
                             client_service);

                continue;
            }
            else if (events[i].events & EPOLLIN)
            {
                int fd = events[i].data.fd;
                char buf[RX_BUF_SIZE];
                ssize_t nread;

                memset(buf, 0, sizeof(buf));

                nread = read(fd, buf, sizeof(buf));
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
                    debug_printf("Connection closed on fd %d\n", fd);

                    if (close(fd) == -1)
                    {
                        sprintf(msg, "close: %s\n", strerror(errno));
                        goto err_loop;
                    }

                    ret = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
                    if (ret == -1)
                    {
                        sprintf(msg, "epoll_ctl: %s\n", strerror(errno));
                        goto err_loop;
                    }

                    continue;
                }

                buf[nread] = '\0';

                debug_printf("Received %ld bytes from fd %d\n", nread, fd);

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
                const char *template = "HTTP/1.1 200 OK\r\n"
                                       "Host: %s:%s\r\n"
                                       "Content-Type: text/plain\r\n"
                                       "Content-Length: 12\r\n"
                                       "Date: %s\r\n"
                                       "Connection: close\r\n"
                                       "\r\n"
                                       "Hello, World!";

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
                    nsend = send(events[i].data.fd, buf, buf_len, MSG_NOSIGNAL);

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
                    else if (nsend == 0)
                    {
                        debug_printf("Connection closed on fd %d\n",
                                     events[i].data.fd);

                        if (close(events[i].data.fd) == -1)
                        {
                            sprintf(msg, "close: %s\n", strerror(errno));
                            goto err_loop;
                        }

                        if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL,
                                      events[i].data.fd, NULL) == -1)
                        {
                            sprintf(msg, "epoll_ctl: %s\n", strerror(errno));
                            goto err_loop;
                        }

                        break;
                    }

                    break;
                }

                free(buf);

                debug_printf("Sent %ld bytes to fd %d\n", nsend,
                             events[i].data.fd);

                ret =
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL);

                if (ret == -1)
                {
                    sprintf(msg, "epoll_ctl (at %s:%d): %s\n", __FILE__,
                            __LINE__, strerror(errno));
                    goto err_loop;
                }

                close(events[i].data.fd);

                debug_printf("Connection closed on fd %d\n", events[i].data.fd);
            }
            else if (events[i].events & (EPOLLERR | EPOLLRDHUP))
            {

                if (close(events[i].data.fd) == -1)
                {
                    sprintf(msg, "close: %s\n", strerror(errno));
                    goto err_loop;
                }

                if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd,
                              NULL) == -1)
                {
                    sprintf(msg, "epoll_ctl: %s\n", strerror(errno));
                    goto err_loop;
                }

                debug_printf("Connection closed on fd %d with error %s (event "
                             "code = %ld)\n",
                             events[i].data.fd,
                             events[i].events & EPOLLERR ? "EPOLLERR"
                                                         : "EPOLLRDHUP",
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
            error_printf(msg);
            sprintf(msg, "close(event.fd=%d): %s\n", events[i].data.fd,
                    strerror(errno));
            goto err_epoll;
        }

        if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL) == -1)
        {
            printf("Failed\n");
            error_printf(msg);
            sprintf(msg, "epoll_ctl(event.fd=%d): %s\n", events[i].data.fd,
                    strerror(errno));
            goto err_epoll;
        }

        debug_printf("Closed connection on fd %d\n", events[i].data.fd);
    }

err_epoll:
    close(epoll_fd);

err_socket:
    close(server_fd);

err_addrinfo:
    printf("Failed\n");
    error_printf(msg);
    ret = EXIT_FAILURE;

exit_with_grace:
    return ret;
}
