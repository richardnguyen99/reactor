/**
 * @file socket.h
 * @author Richard Nguyen (richard@richardhnguyen.com)
 * @brief Socket functions header
 * @version 0.1
 * @date 2023-06-30
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef _JAWS_SOCKET_H
#define _JAWS_SOCKET_H 1

#include "defs.h"

#define LISTEN_QUEUE 1024

/* Convert sock address to host address based on IP */
int tohostname(struct sockaddr *addr, const int family, char *ipstr);

/* Bind socket to host on `port` */
int bindsocket(int port);

#endif // _JAWS_SOCKET_H
