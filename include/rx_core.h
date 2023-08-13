/**
 * @file rx_core.h
 * @author Richard Nguyen (richard@richardhnguyen.com)
 * @brief Global module header for the reactor project
 * @version 0.1
 * @date 2023-08-12
 *
 * @copyright Copyright (c) 2023
 */

#ifndef _RX_CORE_H_
#define _RX_CORE_H_ 1

#include <rx_defs.h>

struct rx_daemon;
struct rx_event;
struct rx_thread_pool;

#define RX_OK    0
#define RX_ERROR -1
#define RX_AGAIN -2
#define RX_DONE  -3
#define RX_UNSET -4

#define CR   (u_char)'\r'
#define LF   (u_char)'\n'
#define CRLF "\r\n"

#define RX_MAX_EVENTS      1024
#define RX_POOL_SIZE       32
#define RX_MAX_THREADS     16
#define RX_MAX_BACKLOG     1024
#define RX_MAX_CONNECTIONS 1024

#if defined(BUFSIZ)
#define RX_BUFSIZE BUFSIZ
#else
#define RX_BUFSIZE 4096
#endif

/* Type aliases */

typedef int rx_status_t;

#include <rx_alloc.h>
#include <rx_daemon.h>

#endif /* _RX_CORE_H_ */
