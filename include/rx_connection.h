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

#ifndef __RX_CONNECTION_H__
#define __RX_CONNECTION_H__ 1

#include <rx_config.h>
#include <rx_core.h>

#define RX_HEADER_BUFFER_SIZE 8192    /* 8KB */
#define RX_BODY_BUFFER_SIZE   1048576 /* 1MB*/

typedef enum rx_connection_state
{
    RX_CONNECTION_STATE_READY,
    RX_CONNECTION_STATE_READING_HEADER,
    RX_CONNECTION_STATE_READING_BODY,
    RX_CONNECTION_STATE_SERVING_REQUEST,
    RX_CONNECTION_STATE_WRITING_RESPONSE,
    RX_CONNECTION_STATE_CLOSING,
    RX_CONNECTION_STATE_CLOSED,
} rx_conn_state_t;

struct rx_connection
{
    /* File descriptor for epoll instance */
    int efd;

    /* Socket from client connection */
    int fd;

    /* Address of the client */
    struct sockaddr addr;

    /* Length of the address */
    socklen_t addr_len;

    /* Client's hostname */
    char host[NI_MAXHOST];

    /* Client's port */
    char port[NI_MAXSERV];

    /* The current state of the connection

        Valid state:
            - `RX_CONNECTION_STATE_READY`: The connection is ready to be used
            - `RX_CONNECTION_STATE_READING`: The connection is reading data
            - `RX_CONNECTION_STATE_WRITING`: The connection is writing data
            - `RX_CONNECTION_STATE_CLOSING`: The connection is closing
            - `RX_CONNECTION_STATE_CLOSED`: The connection is closed and removed
    */
    rx_conn_state_t state;

    size_t task_num;

    /* Buffer to store HTTP Header part

        If the buffer exceeds (RX_HDR_MAXSIZE - 1) bytes, which is 8911,
        the server will return 413 (Request Entity Too Large). */
    char buffer_start[RX_HEADER_BUFFER_SIZE + RX_BODY_BUFFER_SIZE];

    /* Pointer to mark the end of the header */
    char *header_end;

    /* Pointer to mark the beginning of the body */
    char *body_start;

    /* Pointer to mark the end of the body */
    char *buffer_end;

    /* The size of the body buffer for POST/PUT methods

        This field should be only used for POST/PUT methods, and ignored in
        other methods. If a POST/PUT request misses the `Content-Length`
        header, a `400` response should be returned instead.*/
    size_t content_length;

    struct rx_request *request;
    struct rx_response *response;
};

int
rx_connection_init(struct rx_connection *conn, int efd, int fd,
                   struct sockaddr addr, socklen_t addr_len);

void
rx_connection_free(struct rx_connection *conn);

void *
rx_connection_process(struct rx_connection *conn);

#endif /* __RX_CONNECTION_H__ */
