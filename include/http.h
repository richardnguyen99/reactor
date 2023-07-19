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

struct request
{
    char *method;
    char *path;
    char *version;

    char *params;
};

struct request *
http_request_parse(int fd);

#endif // _REACTOR_HTTP_H_
