/**
 * @file http.h
 * @author Richard Nguyen (richard@richardhnguyen.com)
 * @brief Methods and constants to support HTTP
 * @version 0.1
 * @date 2023-07-02
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef _JAWS_HTTP_H
#define _JAWS_HTTP_H 1

#include "defs.h"
#include "hashmap.h"

#define HTTP_SUCCESS 200 // Request is OK
#define HTTP_CREATED 201 // Request has created a new resource

#define HTTP_BADREQUEST 400      // Request is invalid
#define HTTP_UNAUTHORIZED 401    // Request requires user authentication
#define HTTP_FORBIDDEN 403       // Server refuses to respond to request
#define HTTP_NOTFOUND 404        // Server cannot find requested resource
#define HTTP_NOTALLOWED 405      // Request method is not supported
#define HTTP_TIMEOUT 408         // Request took too long to complete
#define HTTP_ENTITIYTOOLARGE 413 // Request header is too large

#define HTTP_INTERNAL 500    // Server encountered an error
#define HTTP_NOTIMPL 501     // Server does not support the functionality
#define HTTP_UNSUPPORTED 505 // Server does not support the HTTP version

#define HTTP_VERSION "HTTP/1.1"

#define HTTP_METHOD_GET (1 << 0)
#define HTTP_METHOD_HEAD (1 << 1)
#define HTTP_METHOD_POST (1 << 2)
#define HTTP_METHOD_PUT (1 << 3)
#define HTTP_METHOD_DELETE (1 << 4)
#define HTTP_METHOD_UNSUPPORTED -1

/*
 Control the number of bytes that this server allows in a request header.
*/
#define HTTP_LIMIT_REQUEST_LINE 8190

#define REQP_DEFAULT 0
#define REQP_COMPLETE 1

extern const char *const endpoints[];

typedef hashmap_t *http_headers_t;
struct request
{
    /**
     * @brief HTTP method
     *
     * https://developer.mozilla.org/en-US/docs/Web/HTTP/Methods
     */
    int method;

    /**
     * @brief HTTP Path name
     *
     * It contains the path part of the request URL.
     */
    char *path;

    /**
     * @brief  HTTP version
     *
     * https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Host
     */
    char version[9];

    /**
     * @brief HTTP body
     *
     * It contains key-value pairs of the request body. It is only available
     * when the request method is POST or PUT.
     */
    char *body;

    /**
     * @brief  HTTP query string
     *
     * It contains key-value pairs of the request query string.
     */
    char *query;

    /**
     * @brief HTTP hostname
     *
     * It contains the hostname header in the request.
     */
    char *hostname;

    /**
     * @brief HTTP port number
     *
     * It contains the port number header in the request.
     */
    char port[7];

    /**
     * @brief HTTP IP address
     *
     * It contains the IP address of the client.
     */
    char ip[INET_ADDRSTRLEN];

    /**
     * @brief HTTP headers
     *
     * It contains the headers of the request.
     */
    http_headers_t headers;

    /**
     * @brief HTTP status code
     *
     * It contains the status code of the response that will be sent back to the
     * client.
     */
    int status;
};

typedef struct request req_t;

int endofhdr(const char *msgbuf, const size_t len);
int endofmsg(const char *msgbuf, const size_t len);

ssize_t readline(int fd, char *msgbuf);

req_t *reqinit(void);
void reqfree(req_t *req);
ssize_t reqread(int fd, req_t *headers);

void reqprint(req_t *req, int print_options);

const char *req_strstatus(int status_code);
void req_send_error(int fd, int status_code);
void req_send_response(int fd, int status);

int handle(int fd, char *ipaddr);

#endif // _JAWS_HTTP_H
