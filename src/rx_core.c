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

int server_fd, client_fd, epoll_fd, i;
socklen_t server_len;
char msg[1024], host[NI_MAXHOST], service[NI_MAXSERV];

struct rx_view rx_view_engine;
struct rx_ring rx_ring_buffer;
struct rx_thread_pool rx_tp;
struct epoll_event ev, events[RX_MAX_EVENTS];
struct sockaddr server;

void
dummy()
{
    printf("Hello, World!\n");
}

void
rx_core_init(int argc, const char **argv)
{
    NOOP(argc);
    NOOP(argv);

    rx_log(LOG_LEVEL_0, LOG_TYPE_INFO, "Initialize core... OK\n");
}

void
rx_core_gai()
{
    int ret;
    const int optval = 1;
    struct addrinfo hints, *res, *p;

    memset(&hints, 0, sizeof(hints));

    hints.ai_family   = AF_INET;     /* IPv4 */
    hints.ai_socktype = SOCK_STREAM; /* TCP */
    hints.ai_flags    = AI_PASSIVE;  /* Listen on all interfaces */

    if (getaddrinfo(NULL, "8080", &hints, &res) != 0)
    {
        rx_log(
            LOG_LEVEL_0, LOG_TYPE_ERROR, "getaddrinfo: %s\n", strerror(errno)
        );

        exit(EXIT_FAILURE);
    }

    for (p = res; p != NULL; p = p->ai_next)
    {
        server_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

        if (server_fd == -1)
        {
            rx_log(LOG_LEVEL_0, LOG_TYPE_WARN, "socket: %s\n", strerror(errno));

            continue;
        }

        ret = setsockopt(
            server_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval)
        );

        if (ret == -1)
        {
            assert(close(server_fd) == 0);
            rx_log(
                LOG_LEVEL_0, LOG_TYPE_WARN, "setsockopt: %s\n", strerror(errno)
            );

            continue;
        }

        ret = bind(server_fd, p->ai_addr, p->ai_addrlen);
        if (ret == -1)
        {
            assert(close(server_fd) == 0);
            rx_log(LOG_LEVEL_0, LOG_TYPE_WARN, "bind: %s\n", strerror(errno));

            continue;
        }

        break;
    }

    if (p == NULL)
    {
        rx_log(LOG_LEVEL_0, LOG_TYPE_ERROR, "Failed to bind socket\n");
        exit(EXIT_FAILURE);
    }

    memcpy(&server, p->ai_addr, p->ai_addrlen);
    server_len = p->ai_addrlen;

    freeaddrinfo(res);

    rx_log(LOG_LEVEL_0, LOG_TYPE_INFO, "Prepare server... OK\n");
}

void
rx_core_gni()
{
    int ret;

    ret = getnameinfo(
        &server, server_len, host, NI_MAXHOST, service, NI_MAXSERV,
        NI_NUMERICSERV
    );

    if (ret != 0)
    {
        rx_log(
            LOG_LEVEL_0, LOG_TYPE_ERROR, "getnameinfo: %s\n", gai_strerror(ret)
        );

        exit(EXIT_FAILURE);
    }
}

void
rx_core_set_nonblocking()
{
    int ret;

    ret = fcntl(server_fd, F_SETFL, O_NONBLOCK);

    if (ret == -1)
    {
        rx_log(LOG_LEVEL_0, LOG_TYPE_ERROR, "fcntl: %s\n", strerror(errno));

        exit(EXIT_FAILURE);
    }
}

void
rx_core_epoll_create()
{
    int ret;

    epoll_fd = epoll_create1(0);

    if (epoll_fd == -1)
    {
        rx_log(
            LOG_LEVEL_0, LOG_TYPE_ERROR, "epoll_create1: %s\n", strerror(errno)
        );

        exit(EXIT_FAILURE);
    }

    ev.events  = EPOLLIN;
    ev.data.fd = server_fd;

    ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev);

    if (ret == -1)
    {
        rx_log(LOG_LEVEL_0, LOG_TYPE_ERROR, "epoll_ctl: %s\n", strerror(errno));

        exit(EXIT_FAILURE);
    }
}

void
rx_core_load_view()
{
    int ret;

    ret = rx_view_init();

    if (ret != RX_OK)
    {
        rx_log(
            LOG_LEVEL_0, LOG_TYPE_ERROR, "rx_view_init: %s\n", strerror(errno)
        );

        exit(EXIT_FAILURE);
    }

    rx_log(LOG_LEVEL_0, LOG_TYPE_INFO, "Load view... OK\n");

    ret = rx_view_load_template("pages/_template.html");

    if (ret != RX_OK)
    {
        rx_log(
            LOG_LEVEL_0, LOG_TYPE_ERROR, "rx_view_load_template: %s\n",
            strerror(errno)
        );

        exit(EXIT_FAILURE);
    }

    rx_log(LOG_LEVEL_0, LOG_TYPE_INFO, "Load template... OK\n");

    ret = rx_view_load_4xx("pages/_4xx.html");

    if (ret != RX_OK)
    {
        rx_log(
            LOG_LEVEL_0, LOG_TYPE_ERROR, "rx_view_load_4xx: %s\n",
            strerror(errno)
        );

        exit(EXIT_FAILURE);
    }

    rx_log(LOG_LEVEL_0, LOG_TYPE_INFO, "Load 4xx... OK\n");

    ret = rx_view_load_5xx("pages/_5xx.html");

    if (ret != RX_OK)
    {
        rx_log(
            LOG_LEVEL_0, LOG_TYPE_ERROR, "rx_view_load_5xx: %s\n",
            strerror(errno)
        );

        exit(EXIT_FAILURE);
    }

    rx_log(LOG_LEVEL_0, LOG_TYPE_INFO, "Load 5xx... OK\n");
}

void
rx_core_load_ring_buffer()
{
    memset(&rx_ring_buffer, 0, sizeof(rx_ring_buffer));
    rx_ring_init(&rx_ring_buffer);

    rx_log(LOG_LEVEL_0, LOG_TYPE_INFO, "Load ring buffer... OK\n");
}

void
rx_core_load_thread_pool()
{
    int ret;

    memset(&rx_tp, 0, sizeof(rx_tp));

    ret = rx_thread_pool_init(&rx_tp, &rx_ring_buffer);

    if (ret != RX_OK)
    {
        rx_log(
            LOG_LEVEL_0, LOG_TYPE_ERROR, "rx_thread_pool_init: %s\n",
            strerror(errno)
        );

        exit(EXIT_FAILURE);
    }

    rx_log(LOG_LEVEL_0, LOG_TYPE_INFO, "Load thread pool... OK\n");
}

void
rx_core_boot()
{
    int ret;

    ret = listen(server_fd, 1024);

    if (ret == -1)
    {
        rx_log(LOG_LEVEL_0, LOG_TYPE_ERROR, "listen: %s\n", strerror(errno));

        exit(EXIT_FAILURE);
    }

    rx_log(LOG_LEVEL_0, LOG_TYPE_INFO, "Make server fd listen... OK\n");
}
