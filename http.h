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

#define HTTP_SUCCESS 200
#define HTTP_CREATED 201

#define HTTP_BADREQUEST 400
#define HTTP_UNAUTHORIZED 401
#define HTTP_FORBIDDEN 403
#define HTTP_NOTFOUND 404
#define HTTP_UNALLOWED 405
#define HTTP_TIMEOUT 408

#define HTTP_INTERNAL 500
#define HTTP_NOTIMPLEMENTED 501

int endofhdr(const char *msgbuf, const size_t len);
int endofmsg(const char *msgbuf, const size_t len);

ssize_t readline(int fd, char *msgbuf);
ssize_t readfrom(int fd);

int handle(int fd);

#endif // _JAWS_HTTP_H
