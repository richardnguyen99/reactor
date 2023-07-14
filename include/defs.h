#define _GNU_SOURCE 1

/**
 * @file defs.h
 * @author Richard Nguyen (richard@richardhnguyen.com)
 * @brief Definitions header file with macros, constants, and functions
 * @version 0.1.1
 * @date 2023-07-14
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef _REACTOR_DEFS_H
#define _REACTOR_DEFS_H 1

// ISO C
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <signal.h>

// POSIX
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

// Linux
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/mman.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>

// Macros & Constants
#define REACTOR_VERSION "0.1.1"
#define REACTOR_VERSION_MAJOR 0
#define REACTOR_VERSION_MINOR 1
#define REACTOR_VERSION_PATCH 1

#define DEFAULT_PORT 9999
#define DEFAULT_ROOTDIR "public/"
#define DEFAULT_NTHREADS 4

#define MAX_EVENTS 1024
#define BUFSIZE 1024
#define MSGSIZE 8192

#ifdef DEBUG
#define debug(fmt, ...) fprintf(stdout, fmt, ##__VA_ARGS__)
#else
#define debug(fmt, ...)
#endif


#endif // _REACTOR_DEFS_H
