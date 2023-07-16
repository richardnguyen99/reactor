/**
 * @file http.h
 * @author Richard Nguyen (richard@richardhnguyen.com)
 * @brief HTTP header file
 * @version 0.1
 * @date 2023-07-16
 * 
 * @copyright Copyright (c) 2023
 */

#ifndef _REACTOR_HTTP_H
#define _REACTOR_HTTP_H 1

#include "defs.h"
#include "hashmap.h"

enum HTTP_METHOD
{
    HTTP_METHOD_GET = 0x001,
#define HTTP_METHOD_GET HTTP_METHOD_GET
    HTTP_METHOD_POST = 0x002,
#define HTTP_METHOD_POST HTTP_METHOD_POST
    HTTP_METHOD_DELETE = 0x004,
#define HTTP_METHOD_DELETE HTTP_METHOD_DELETE
    HTTP_METHOD_HEAD = 0x008,
#define HTTP_METHOD_HEAD HTTP_METHOD_HEAD
}; 

struct request 
{
    int method;
    char base_url[NAME_MAX];
    char path[PATH_MAX];
    char hostname[NAME_MAX];
    char ip[INET_ADDRSTRLEN];
    char params[NAME_MAX];

    struct hashmap *headers;
};

int http_req(int fd);



#endif // _REACTOR_HTTP_H
