/**
 * @file http.h
 * @author Richard Nguyen (richard@richardhnguyen.com)
 * @brief HTTP class header file.
 * @version 0.1
 * @date 2023-07-18
 *
 * @copyright Copyright (c) 2023
 */

#ifndef _REACTOR_HTTP_H_
#define _REACTOR_HTTP_H_ 1

#include "defs.h"
#include "dict.h"
#include "util.h"

// clang-format off
#define HTTP_ERROR                 -1
#define HTTP_NOT_SET                0
#define HTTP_READ_AGAIN             1

#define HTTP_SUCCESS                200
#define HTTP_CREATED                201

#define HTTP_MOVED_PERMANENTLY      301

#define HTTP_BAD_REQUEST            400
#define HTTP_UNAUTHORIZED           401
#define HTTP_FORBIDDEN              403
#define HTTP_NOT_FOUND              404
#define HTTP_METHOD_NOT_ALLOWED     405
#define HTTP_REQUEST_TIMEOUT        408

#define HTTP_INTERNAL_SERVER_ERROR  500
#define HTTP_NOT_IMPLEMENTED        501
#define HTTP_BAD_GATEWAY            502
#define HTTP_SERVICE_UNAVAILABLE    503
#define HTTP_VERSION_NOT_SUPPORTED  505

#define HTTP_SUCCESS_MSG                "OK"
#define HTTP_CREATED_MSG                "Created"

#define HTTP_MOVED_PERMANENTLY_MSG      "Moved Permanently"

#define HTTP_BAD_REQUEST_MSG            "Bad Request"
#define HTTP_UNAUTHORIZED_MSG           "Unauthorized"
#define HTTP_FORBIDDEN_MSG              "Forbidden"
#define HTTP_NOT_FOUND_MSG              "Not Found"
#define HTTP_METHOD_NOT_ALLOWED_MSG     "Method Not Allowed"
#define HTTP_REQUEST_TIMEOUT_MSG        "Request Timeout"

#define HTTP_INTERNAL_SERVER_ERROR_MSG  "Internal Server Error"
#define HTTP_NOT_IMPLEMENTED_MSG        "Not Implemented"
#define HTTP_BAD_GATEWAY_MSG            "Bad Gateway"
#define HTTP_SERVICE_UNAVAILABLE_MSG    "Service Unavailable"
// clang-format on

#define GET_HTTP_MSG(status)                                                   \
    (status == HTTP_SUCCESS)                 ? HTTP_SUCCESS_MSG                \
    : (status == HTTP_CREATED)               ? HTTP_CREATED_MSG                \
    : (status == HTTP_MOVED_PERMANENTLY)     ? HTTP_MOVED_PERMANENTLY_MSG      \
    : (status == HTTP_BAD_REQUEST)           ? HTTP_BAD_REQUEST_MSG            \
    : (status == HTTP_UNAUTHORIZED)          ? HTTP_UNAUTHORIZED_MSG           \
    : (status == HTTP_FORBIDDEN)             ? HTTP_FORBIDDEN_MSG              \
    : (status == HTTP_NOT_FOUND)             ? HTTP_NOT_FOUND_MSG              \
    : (status == HTTP_METHOD_NOT_ALLOWED)    ? HTTP_METHOD_NOT_ALLOWED_MSG     \
    : (status == HTTP_REQUEST_TIMEOUT)       ? HTTP_REQUEST_TIMEOUT_MSG        \
    : (status == HTTP_INTERNAL_SERVER_ERROR) ? HTTP_INTERNAL_SERVER_ERROR_MSG  \
    : (status == HTTP_NOT_IMPLEMENTED)       ? HTTP_NOT_IMPLEMENTED_MSG        \
    : (status == HTTP_BAD_GATEWAY)           ? HTTP_BAD_GATEWAY_MSG            \
    : (status == HTTP_SERVICE_UNAVAILABLE)   ? HTTP_SERVICE_UNAVAILABLE_MSG    \
                                             : NULL

#define HTTP_METHOD_INVALID 0
#define HTTP_METHOD_GET     (1 << 0)
#define HTTP_METHOD_HEAD    (1 << 1)
#define HTTP_METHOD_POST    (1 << 2)
#define HTTP_METHOD_PUT     (1 << 3)
#define HTTP_METHOD_DELETE  (1 << 4)

#define GET_HTTP_METHOD(method)                                                \
    (method == HTTP_METHOD_GET)      ? "GET"                                   \
    : (method == HTTP_METHOD_HEAD)   ? "HEAD"                                  \
    : (method == HTTP_METHOD_POST)   ? "POST"                                  \
    : (method == HTTP_METHOD_PUT)    ? "PUT"                                   \
    : (method == HTTP_METHOD_DELETE) ? "DELETE"                                \
                                     : NULL

#endif // _REACTOR_HTTP_H_
