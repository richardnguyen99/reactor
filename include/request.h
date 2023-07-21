/**
 * @file request.h
 * @author Richard Nguyen (richard@richardhnguyen.com)
 * @brief HTTP Request handlers for the server header file
 * @version 0.1
 * @date 2023-07-19
 *
 * @copyright Copyright (c) 2023
 */

#ifndef _REACTOR_REQUEST_H_
#define _REACTOR_REQUEST_H_ 1

#include "defs.h"

struct request
{
    int method;
    char path[BUFSIZ];
    char version[HTTP_VER_SIZE];

    char *raw;
    size_t len;
    size_t cap;
};

struct request *
request_new();

void
request_free(struct request *request);

#endif // _REACTOR_REQUEST_H_
