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
rx_connection_free(struct rx_connection *conn)
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

    conn->request = NULL;

    close(conn->fd);
}
