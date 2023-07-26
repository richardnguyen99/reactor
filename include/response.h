/**
 * @file response.h
 * @author Richard Nguyen (richard@richardhnguyen.com)
 * @brief HTTP Response handlers for the server header file
 * @version 0.1
 * @date 2023-07-19
 *
 * @copyright Copyright (c) 2023
 */

#ifndef _REACTOR_RESPONSE_H_
#define _REACTOR_RESPONSE_H_ 1

#include "defs.h"
#include "dict.h"
#include "http.h"

struct response
{
    int status;
    int method;

    struct dict *headers;
    char *body;
    size_t body_len;
};

struct response *
response_new();

void
response_construct(struct response *res, int status, int method,
                   const char *filename);

ssize_t
response_send(struct response *response, int fd);

void
response_free(struct response *response);

#endif // _REACTOR_RESPONSE_H_
