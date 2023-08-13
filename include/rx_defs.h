/**
 * @file rx_defs.h
 * @author Richard Nguyen (richard@richardhnguyen.com)
 * @brief Reactor definition headers and macros
 * @version 0.1
 * @date 2023-08-12
 *
 * @copyright Copyright (c) 2023
 *
 * @note Global header configuration for the reactor project. You might want to
 * import this header in other files to get access to all the functionalities
 * used in the project.
 */

#ifndef _RX_DEFS_H_
#define _RX_DEFS_H_ 1

/* C Standard Library */
#include <assert.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Posix library */
#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/mman.h>
#include <sys/signalfd.h>
#include <sys/stat.h>
#include <sys/timerfd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/* Network and Socket library */
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>

/* Replacable headers */

#if defined(RX_HAS_LIMITS_H)
#include <limits.h>
#include <linux/limits.h>
#else
#define NR_OPEN 1024

#define NGROUPS_MAX    65536  /* supplemental group IDs are available */
#define ARG_MAX        131072 /* # bytes of args + environ for exec() */
#define LINK_MAX       127    /* # links a file may have */
#define MAX_CANON      255    /* size of the canonical input queue */
#define MAX_INPUT      255    /* size of the type-ahead buffer */
#define NAME_MAX       255    /* # chars in a file name */
#define PATH_MAX       4096   /* # chars in a path name including nul */
#define PIPE_BUF       4096   /* # bytes in atomic write to a pipe */
#define XATTR_NAME_MAX 255    /* # chars in an extended attribute name */
#define XATTR_SIZE_MAX 65536  /* size of an extended attribute value (64k) */
#define XATTR_LIST_MAX 65536  /* size of extended attribute namelist (64k) */

#define RTSIG_MAX 32
#endif

#if defined(RX_HAS_MATH_H)
#include <math.h>
#endif

/* Global macros */

#define rx_noop(x) ((void)(x))

#define rx_max(a, b)                                                           \
    ({                                                                         \
        __typeof__(a) _a = (a);                                                \
        __typeof__(b) _b = (b);                                                \
        _a > _b ? _a : _b;                                                     \
    })

#define rx_min(a, b)                                                           \
    ({                                                                         \
        __typeof__(a) _a = (a);                                                \
        __typeof__(b) _b = (b);                                                \
        _a < _b ? _a : _b;                                                     \
    })

#define rx_abs(a)                                                              \
    ({                                                                         \
        __typeof__(a) _a = (a);                                                \
        _a < 0 ? -_a : _a;                                                     \
    })

#define rx_len(a)                                                              \
    ({                                                                         \
        __typeof__(a) _a = (a);                                                \
        sizeof(_a) / sizeof(_a[0]);                                            \
    })

#endif /* _RX_DEFS_H_ */
