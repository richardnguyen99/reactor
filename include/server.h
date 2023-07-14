/**
 * @file server.h
 * @author Richard Nguyen (richard@richardhnguyen.com)
 * @brief Server header file
 * @version 0.1.1
 * @date 2023-07-14
 *
 * @copyright Copyright (c) 2023
 */

#ifndef _REACTOR_SERVER_H
#define _REACTOR_SERVER_H 1

#include "defs.h"

struct server {
  int sockfd;
  int epollfd;
  int nthreads;
  int port;
  char rootdir[8192];
  struct epoll_event *events;
  struct sockaddr_in addr;
  pthread_t *threads;
};

typedef struct server server_t;

void server_print(void);

#endif // _REACTOR_SERVER_H
