/**
 * @file defs.h
 * @author Richard Nguyen (richard@richardhnguyen.com)
 * @brief Common C headers that will be used in this project
 * @version 0.1
 * @date 2023-06-30
 *
 * @copyright Copyright (c) 2023
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#ifndef _JAWS_DEFS_H
#define _JAWS_DEFS_H 1

// ISO standard headers
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// POSIX standard headers
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>

// Linux-related interface headers
#include <sys/epoll.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>

#define FAILURE 1
#define SUCCESS 0
#define ERROR -1
#define FATAL -2

#define MINPORT 1025     /* Minimum port range */
#define MAXPORT 65536    /* Maximum port range */
#define DEFAULTPORT 9999 /* Default port number */

#define MINNOTHREADS 2     /* Minimum number of threads */
#define MAXNOTHREADS 10    /* Maximum number of threads */
#define DEFAULTNOTHREADS 2 /* Default number of threads */

#define RSRCSIZE 128 /* Resource size for HTTP request */
#define BUFSIZE 1024 /* Normal buffer size for small reading */
#define MSGSIZE 8096 /* Message size for HTTP request */

// Custom printf only prints out the message on debug mode.
#ifdef DEBUG
#define dprintf(file, fmt, ...) fprintf(file, fmt, ##__VA_ARGS__)
#else
#define dprintf(file, fmt, ...)
#endif

struct configopt
{
    char root[80];     // Root directory to serve the content
    size_t port;       // Port number to bind host.
    size_t num_thread; // Available number of threads in a pool
};

extern struct configopt conf;

#endif // _JAWS_DEFS_H
