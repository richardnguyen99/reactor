/**
 * @file defs.h
 * @author Richard Nguyen (richard@richardhnguyen.com)
 * @brief Definitions header file with macros, constants, and functions
 * @version 0.2.0
 * @date 2023-07-23
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef _REACTOR_DEFS_H
#define _REACTOR_DEFS_H 1

// ISO C
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// POSIX
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// Linux
#include <arpa/inet.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/mman.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/timerfd.h>

// JSON-C
#include <json-c/json.h>

#ifdef HAVE_LIMITS_H
#include <linux/limits.h>
#else
#define PATH_MAX 4096
#define NAME_MAX 255
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef SIZE
#define SIZE(a) (sizeof(a) / sizeof(a[0]))
#endif

#ifndef SUCCESS
#define SUCCESS 0
#endif

#ifndef FAILURE
#define FAILURE 1
#endif

#ifndef ERROR
#define ERROR -1
#endif

#ifndef DIE
#define DIE(msg)                                                               \
    do                                                                         \
    {                                                                          \
        perror(msg);                                                           \
        exit(EXIT_FAILURE);                                                    \
    } while (0)
#endif

#ifndef DIEASYOUWISH
#define DIEASYOUWISH(msg)                                                      \
    do                                                                         \
    {                                                                          \
        fprintf(stderr, msg);                                                  \
        exit(EXIT_FAILURE);                                                    \
    } while (0)
#endif

// Macros & Constants
#define REACTOR_VERSION       "0.2.0"
#define REACTOR_VERSION_MAJOR 0
#define REACTOR_VERSION_MINOR 2
#define REACTOR_VERSION_PATCH 0

#define DEFAULT_PORT     9999
#define DEFAULT_ROOTDIR  "public/"
#define DEFAULT_NTHREADS 4

#define MAX_EVENTS    1024
#define MSGSIZE       8192
#define PRTSIZ        6
#define HTTP_VER_SIZE 10

#ifndef BUFSIZ
#define BUFSIZ 8192
#endif

#ifdef DEBUG
#define debug(fmt, ...) fprintf(stdout, fmt, ##__VA_ARGS__)
#else
#define debug(fmt, ...)
#endif

#endif // _REACTOR_DEFS_H
