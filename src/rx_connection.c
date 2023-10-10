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

int
rx_connection_init(struct rx_connection *conn, int efd, int fd,
                   struct sockaddr addr, socklen_t addr_len)
{
    conn->efd      = efd;
    conn->fd       = fd;
    conn->addr_len = addr_len;
    conn->request  = NULL;
    conn->response = NULL;

    memset(conn->buffer_start, 0, sizeof(conn->buffer_start));
    memcpy(&conn->addr, &addr, addr_len);

    conn->buffer_end = conn->buffer_start;
    conn->header_end = conn->buffer_start;
    conn->body_start = conn->buffer_start;

    if (getnameinfo(&conn->addr, conn->addr_len, conn->host, NI_MAXHOST,
                    conn->port, NI_MAXSERV, NI_NUMERICSERV) != 0)
    {
        rx_log_error("getnameinfo() failed: %s", strerror(errno));
        return RX_ERROR;
    }

    conn->state    = RX_CONNECTION_STATE_READY;
    conn->task_num = 0;

    return RX_OK;
}

void
rx_connection_cleanup(struct rx_connection *conn)
{
    if (conn->request != NULL)
    {
        conn->request->state = RX_REQUEST_STATE_DONE;

        rx_request_destroy(conn->request);
        free(conn->request);
    }

    if (conn->response != NULL)
    {
        conn->response->status_code = RX_HTTP_STATUS_CODE_UNSET;

        rx_response_destroy(conn->response);
        free(conn->response);
    }

    conn->request  = NULL;
    conn->response = NULL;
}

void
rx_connection_free(struct rx_connection *conn)
{
    rx_connection_cleanup(conn);

    close(conn->fd);
}

void *
rx_connection_process(struct rx_connection *conn)
{
    pthread_t tid = pthread_self();
    int ret;
    char *startl, *endl;
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

    startl = endl + 2;
    endl   = strstr(startl, "\r\n");
    ret    = rx_request_process_headers(conn->request, startl, endl - startl);

    conn->state = RX_CONNECTION_STATE_WRITING_RESPONSE;

    end = clock();

    rx_log(LOG_LEVEL_0, LOG_TYPE_INFO,
           "[Thread %ld]%4.sRequest processed successfully (took %.4fms)"
           "\n",
           tid, "", (double)(end - start) / CLOCKS_PER_SEC * 1000);

    ret = rx_route_get(
        &route, conn->request->uri.path,
        (size_t)(conn->request->uri.path_end - conn->request->uri.path));

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
    else
    {
        rx_route_4xx(conn->request, conn->response,
                     RX_HTTP_STATUS_CODE_NOT_FOUND);

        goto end;
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
