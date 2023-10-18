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
struct rx_thread_pool;
struct rx_view;

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
    RX_HTTP_STATUS_CODE_UNSET                  = 0,
    RX_HTTP_STATUS_CODE_OK                     = 200,
    RX_HTTP_STATUS_CODE_FOUND                  = 302,
    RX_HTTP_STATUS_CODE_BAD_REQUEST            = 400,
    RX_HTTP_STATUS_CODE_NOT_FOUND              = 404,
    RX_HTTP_STATUS_CODE_METHOD_NOT_ALLOWED     = 405,
    RX_HTTP_STATUS_CODE_UNSUPPORTED_MEDIA_TYPE = 415,
    RX_HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR  = 500,
};

#define RX_HTTP_STATUS_MSG_UNSET                  "Unset"
#define RX_HTTP_STATUS_MSG_OK                     "OK"
#define RX_HTTP_STATUS_MSG_FOUND                  "Found"
#define RX_HTTP_STATUS_MSG_BAD_REQUEST            "Bad Request"
#define RX_HTTP_STATUS_MSG_NOT_FOUND              "Not Found"
#define RX_HTTP_STATUS_MSG_METHOD_NOT_ALLOWED     "Method Not Allowed"
#define RX_HTTP_STATUS_MSG_UNSUPPORTED_MEDIA_TYPE "Unsupported Media Type"
#define RX_HTTP_STATUS_MSG_INTERNAL_SERVER_ERROR  "Internal Server Error"

/* Should be used when parsing requests or reading files for fast comparison. */

enum rx_http_mime_enum
{
    RX_HTTP_MIME_NONE              = 0x000000000,
    RX_HTTP_MIME_ALL               = 0x000000FFF,
    RX_HTTP_MIME_TEXT_ALL          = 0x000000008,
    RX_HTTP_MIME_TEXT_PLAIN        = 0x000000009,
    RX_HTTP_MIME_TEXT_HTML         = 0x00000000A,
    RX_HTTP_MIME_TEXT_CSS          = 0x00000000B,
    RX_HTTP_MIME_TEXT_JS           = 0x00000000C,
    RX_HTTP_MIME_TEXT_OCTET_STREAM = 0x00000000D,
    RX_HTTP_MIME_APPLICATION_ALL   = 0x000000010,
    RX_HTTP_MIME_APPLICATION_XML   = 0x000000011,
    RX_HTTP_MIME_APPLICATION_JSON  = 0x000000012,
    RX_HTTP_MIME_APPLICATION_XHTML = 0x000000013,
    RX_HTTP_MIME_APPLICATION_XFORM = 0x000000014,
    RX_HTTP_MIME_IMAGE_ALL         = 0x000000020,
    RX_HTTP_MIME_IMAGE_ICO         = 0x000000021,
    RX_HTTP_MIME_IMAGE_GIF         = 0x000000022,
    RX_HTTP_MIME_IMAGE_JPEG        = 0x000000023,
    RX_HTTP_MIME_IMAGE_PNG         = 0x000000024,
    RX_HTTP_MIME_IMAGE_SVG         = 0x000000025,
};

/* Should be used when constructing a response. Therefore, no all media-types
   are supported. */

#define RX_HTTP_MIME_NONE_STR              ""
#define RX_HTTP_MIME_ALL_STR               "*/*"
#define RX_HTTP_MIME_TEXT_ALL_STR          "text/*"
#define RX_HTTP_MIME_TEXT_PLAIN_STR        "text/plain;charset=utf-8"
#define RX_HTTP_MIME_TEXT_HTML_STR         "text/html;charset=utf-8"
#define RX_HTTP_MIME_TEXT_CSS_STR          "text/css;charset=utf-8"
#define RX_HTTP_MIME_TEXT_JS_STR           "text/javascript;charset=utf-8"
#define RX_HTTP_MIME_TEXT_OCTET_STREAM_STR "text/octet-stream"
#define RX_HTTP_MIME_APPLICATION_ALL       "application/*"
#define RX_HTTP_MIME_APPLICATION_XML_STR   "application/xml"
#define RX_HTTP_MIME_APPLICATION_JSON_STR  "application/json"
#define RX_HTTP_MIME_APPLICATION_XHTML_STR "application/xhtml+xml"
#define RX_HTTP_MIME_APPLICATION_XFORM_STR "application/x-www-form-urlencoded"
#define RX_HTTP_MIME_IMAGE_ALL_STR         "image/*"
#define RX_HTTP_MIME_IMAGE_ICO_STR         "image/x-icon"
#define RX_HTTP_MIME_IMAGE_GIF_STR         "image/gif"
#define RX_HTTP_MIME_IMAGE_JPEG_STR        "image/jpeg"
#define RX_HTTP_MIME_IMAGE_PNG_STR         "image/png"
#define RX_HTTP_MIME_IMAGE_SVG_STR         "image/svg+xml"

#define RX_HTTP_MIME_TO_STR(mime) mime##_STR

#define RX_MAX_URI_LENGTH 2048

typedef enum rx_http_status_enum rx_http_status_t;
typedef enum rx_http_mime_enum rx_http_mime_t;

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
#include <rx_thread.h>
#include <rx_view.h>

extern int server_fd, client_fd, epoll_fd, n, i;
extern socklen_t server_len;
extern char msg[1024], host[NI_MAXHOST], service[NI_MAXSERV];

extern struct rx_view rx_view_engine;
extern struct rx_ring rx_ring_buffer;
extern struct rx_thread_pool rx_tp;
extern struct epoll_event ev, events[RX_MAX_EVENTS];
extern struct sockaddr server;

void
rx_core_init(int argc, const char **argv);

void
rx_core_gai();

void
rx_core_gni();

void
rx_core_set_nonblocking();

void
rx_core_epoll_create();

void
rx_core_load_view();

void
rx_core_load_ring_buffer();

void
rx_core_load_thread_pool();

void
rx_core_boot();

#endif /* __RX_CORE_H__ */
