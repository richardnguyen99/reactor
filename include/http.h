/**
 * @file http.h
 * @author Richard Nguyen (richard@richardhnguyen.com)
 * @brief HTTP class for all http functionalities.
 * @version 0.2
 * @date 2023-08-07
 *
 * @copyright Copyright (c) 2023
 */

#ifndef _REACTOR_HTTP_H_
#define _REACTOR_HTTP_H_ 1

#include "dict.h"
#include "httpdef.h"
#include "request.h"
#include "response.h"
#include "util.h"

struct reactor_socket;
struct reactor;

struct http_obj
{

    struct request *req;
    struct response *res;

    int cfd;
};

struct http_obj *
http_new();

int
http_request_line(struct http_obj *http);

int
http_request_headers(struct http_obj *http);

void
http_response_status(struct http_obj *http, int status);

void
http_free(struct http_obj *http);

char *
http_get_status_text(int status);

typedef int (*http_header_handler)(const char *);

/**
 * @brief
 *
 * @param headers
 * @param key
 * @return int
 */
int
http_require_header(struct dict *headers, const char *key,
                    http_header_handler func);

struct dict *
http_require_accept(struct dict *headers);

#endif // _REACTOR_HTTP_H_
