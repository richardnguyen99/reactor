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
rx_connection_init(
    struct rx_connection *conn, int efd, int fd, struct sockaddr addr,
    socklen_t addr_len
)
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

    if (getnameinfo(
            &conn->addr, conn->addr_len, conn->host, NI_MAXHOST, conn->port,
            NI_MAXSERV, NI_NUMERICSERV
        ) != 0)
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
    conn->task_num = 0;
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

    rx_log(
        LOG_LEVEL_0, LOG_TYPE_INFO,
        "[Thread %ld]%4.sProcessing request from %s:%s (socket = %d)\n", tid,
        "", conn->host, conn->port, conn->fd
    );

    if (conn->state != RX_CONNECTION_STATE_SERVING_REQUEST)
    {
        rx_log(
            LOG_LEVEL_0, LOG_TYPE_ERROR,
            "Connection is not ready to serve request\n"
        );
        return RX_ERROR_PTR;
    }

    start  = clock();
    startl = conn->buffer_start;
    endl   = startl;

    memset(&route, 0, sizeof(route));

    rx_log(
        LOG_LEVEL_0, LOG_TYPE_DEBUG, "[Thread %ld]%4.sHeader length: %ld\n",
        tid, "", conn->header_end - conn->buffer_start
    );

    /*
       Process the request buffer can be divided into 3 parts:
           - Request start line (1)
           - Request headers (2)
           - Request body (only POST requires this part) (3)
     */

    endl = strstr(startl, "\r\n");

    /* Process the request start line (1)

       An example of a request start line:

       ```
           GET  /                  HTTP/1.1\r\n
           POST /login             HTTP/1.1\r\n
           HEAD /public/index.html HTTP/1.1\r\n
       ```

         The request start line contains 3 parts:
              - Request method
              - Request URI
              - Request version
     */

    ret = rx_request_process_start_line(conn->request, startl, endl - startl);
    if (ret != RX_OK)
    {
        rx_log(
            LOG_LEVEL_0, LOG_TYPE_ERROR,
            "[Thread %ld]%4.sFailed to process request start line\n", tid, ""
        );
        return RX_ERROR_PTR;
    }

    /* Skip CRLF and go to the next line */

    startl = endl + 2;
    endl   = strstr(startl, "\r\n");

    /* Process the headers (2)

       An example of request headers:

       ```
              Host: localhost:8080\r\n
              User-Agent: curl/7.68.0\r\n
              Accept: ...\r\n
              Content-Length: 0\r\n
              Content-Type: application/x-www-form-urlencoded\r\n
              ...
              \r\n
       ```
     */

    ret = rx_request_process_headers(conn->request, startl, endl - startl);

    if (ret != RX_OK)
    {
        rx_route_4xx(
            conn->request, conn->response, RX_HTTP_STATUS_CODE_BAD_REQUEST
        );

        goto end;
    }

    end = clock();

    rx_log(
        LOG_LEVEL_0, LOG_TYPE_INFO,
        "[Thread %ld]%4.sRequest processed successfully (took %.4fms)"
        "\n",
        tid, "", (double)(end - start) / CLOCKS_PER_SEC * 1000
    );

    /* HTTP/1.1 REQUIREs request message to contain Host header, although this
       information is not used any further in the server except for checking.
     */

    if (conn->request->host.result > RX_REQUEST_HEADER_HOST_RESULT_OK)
    {
        rx_route_4xx(
            conn->request, conn->response, RX_HTTP_STATUS_CODE_BAD_REQUEST
        );

        goto end;
    }

    /* Get route handler from the router table based on the path that has been
       parsed from the request start line.

       If the route is not found, return 404 Not Found.
     */

    ret = rx_route_get(
        &route, conn->request->uri.path,
        (size_t)(conn->request->uri.path_end - conn->request->uri.path)
    );

    if (ret != RX_OK)
    {
        rx_route_4xx(
            conn->request, conn->response, RX_HTTP_STATUS_CODE_NOT_FOUND
        );
        goto end;
    }

    /* Route handler based on the request method

       If the route is found, it will store the handler in the `route`
       structure. The handler contains a function pointer that handles the
       request based on the request method.

       If the request method is not supported, return 405 (Method Not Allowed).
     */

    if (conn->request->method == RX_REQUEST_METHOD_GET /***/
        && route.handler.get != NULL)
    {
        route.handler.get(conn->request, conn->response);
    }
    else if (conn->request->method == RX_REQUEST_METHOD_POST /**/
             && route.handler.post != NULL)
    {

        /* Process the body of request (3)

           A valid body in the request requires 2 things:
               - Content-Length header (> 0)
               - Content-Type header (supported types:
                 application/x-www-form-urlencoded, application/json)
         */

        if (conn->buffer_end > conn->body_start)
        {
            rx_log(
                LOG_LEVEL_0, LOG_TYPE_INFO,
                "[Thread %ld]%4.sBody is found in request (length = %ld)\n",
                tid, "", conn->buffer_end - conn->body_start
            );

            /* Check if the content length matches with the actual body length

               If the content length is invalid, return 400 Bad Request.
             */
            if (conn->request->content_length == 0 ||
                ((size_t)(conn->buffer_end - conn->body_start) !=
                 conn->request->content_length))
            {
                rx_route_4xx(
                    conn->request, conn->response,
                    RX_HTTP_STATUS_CODE_BAD_REQUEST
                );

                goto end;
            }

            /* Check if the content type is one of the supported types

               If the content type is not matched with any of the supported
               types, return 415 (Unsupported Media Type).
             */
            switch (conn->request->content_type)
            {
            case RX_HTTP_MIME_APPLICATION_XFORM:
                conn->request->content = conn->body_start;
                break;

            default:
                rx_route_4xx(
                    conn->request, conn->response,
                    RX_HTTP_STATUS_CODE_UNSUPPORTED_MEDIA_TYPE
                );

                goto end;
            }
        }

        route.handler.post(conn->request, conn->response);
    }
    else if (conn->request->method == RX_REQUEST_METHOD_PUT /**/
             && route.handler.put != NULL)
    {
        route.handler.put(conn->request, conn->response);
    }
    else if (conn->request->method == RX_REQUEST_METHOD_DELETE /**/
             && route.handler.delete != NULL)
    {
        route.handler.delete(conn->request, conn->response);
    }
    else if (conn->request->method == RX_REQUEST_METHOD_HEAD /**/
             && route.handler.head != NULL)
    {
        route.handler.head(conn->request, conn->response);
    }
    else
    {
        rx_route_4xx(
            conn->request, conn->response,
            RX_HTTP_STATUS_CODE_METHOD_NOT_ALLOWED
        );

        goto end;
    }

end:
    (void)rx_response_construct(conn->response);

    return RX_OK_PTR;
}
