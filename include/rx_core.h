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

#ifndef __RX_CORE_H__
#define __RX_CORE_H__ 1

#include <rx_config.h>

struct rx_log;
struct rx_event;
struct rx_server;
struct rx_client;
struct rx_request;
struct rx_response;
struct rx_connection;
struct rx_string;
struct rx_file;
struct rx_request_uri;
struct rx_task;
struct rx_ring;
struct rx_qlist;
struct rx_route;

typedef struct rx_string rx_str_t;

#define RX_OK               0
#define RX_ERROR            -1
#define RX_FATAL_WITH_ERROR -2
#define RX_ALLOC_FAILED     -3
#define RX_AGAIN            -4

#define RX_OK_PTR           ((void *)RX_OK)
#define RX_ERROR_PTR        ((void *)RX_ERROR)
#define RX_FATAL_PTR        ((void *)RX_FATAL_WITH_ERROR)
#define RX_ALLOC_FAILED_PTR ((void *)RX_ALLOC_FAILED)
#define RX_AGAIN_PTR        ((void *)RX_AGAIN)

enum rx_http_status_enum
{
    RX_HTTP_STATUS_CODE_UNSET                 = 0,
    RX_HTTP_STATUS_CODE_OK                    = 200,
    RX_HTTP_STATUS_CODE_BAD_REQUEST           = 400,
    RX_HTTP_STATUS_CODE_NOT_FOUND             = 404,
    RX_HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR = 500,
};

#define RX_HTTP_STATUS_MSG_OK                    "OK"
#define RX_HTTP_STATUS_MSG_BAD_REQUEST           "Bad Request"
#define RX_HTTP_STATUS_MSG_NOT_FOUND             "Not Found"
#define RX_HTTP_STATUS_MSG_INTERNAL_SERVER_ERROR "Internal Server Error"

#define RX_MAX_URI_LENGTH 2048

typedef enum rx_http_status_enum rx_http_status_t;

#include <rx_connection.h>
#include <rx_file.h>
#include <rx_log.h>
#include <rx_qlist.h>
#include <rx_request.h>
#include <rx_response.h>
#include <rx_ring.h>
#include <rx_route.h>
#include <rx_string.h>
#include <rx_task.h>

#endif /* __RX_CORE_H__ */
