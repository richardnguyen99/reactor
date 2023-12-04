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

/* The connection structure

   The `rx_connection` represents a state object between a client and the
   server. The structure contains a request and response structure, as well as
   information about the client that is needed to process the request and
   construct the response successfully.
 */
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

        If the buffer exceeds (RX_HDR_MAXSIZE) bytes, which is 8912,
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

/* Initialize and establish connection between a client and the server

   This function is used when a new client arrives (detected by the server via
   the event loop) to initialize a new connection. Each connection represents
   a communication channel between one client and the server at a time.
 */
int
rx_connection_init(
    struct rx_connection *conn, int efd, int fd, struct sockaddr addr,
    socklen_t addr_len
);

/* Deallocate and free memory of a connection

   This function is used by the server to deallocate and free memory of a
   connection when the communication is closed (either a successful EPOLLOUT or
   an error).
 */
void
rx_connection_free(struct rx_connection *conn);

/* Reset the state of a connection

   This function is used to reset the state of connection. Instead of freeing
   and reallocating new memory, the server can reuse the connection object when
   the communication is done but the connection is still alive (keep-alive).
 */
void
rx_connection_cleanup(struct rx_connection *conn);

/* Dispatch a connection

   This function is used in the thread pool by a consumer thread to process the
   connection. This function should be called after the connection has fully
   received the request buffer from the client.

   After processing request, the function will continue to construct a response
   based on the request and store the response into a buffer. Then, it will
   notify the event loop to write the response back to the client.
 */
void *
rx_connection_process(struct rx_connection *conn);

#endif /* __RX_CONNECTION_H__ */
