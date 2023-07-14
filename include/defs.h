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

// POSIX
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

// Linux
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/mman.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>

#endif // _REACTOR_DEFS_H
