/**
 * @file poll.h
 * @author Richard Nguyen (richard@richardhnguyen.com)
 * @brief Polling header file
 * @version 0.1
 * @date 2023-07-15
 * 
 * @copyright Copyright (c) 2023
 */

#ifndef _REACTOR_POLL_H
#define _REACTOR_POLL_H 1

#include "defs.h"

/**
 * @brief Create a epoll instance
 * 
 * @param flags Flags to be configured
 * @return File descriptor of the epoll instance. -1 is returned on error, otherwise.
 */
int poll_create(int flags);

/**
 * @brief Add a file descriptor to the epoll instance for monitoring
 * 
 * @param poll_fd File descriptor for the epoll instance
 * @param fd File descriptor to be monitored
 * @param events Events to be monitored
 * @param poll_events Event queue from the server
 * @return 0 on success, -1 on error
 */
int poll_add(int poll_fd, int fd, int events);

/**
 * @brief  Remove a file descriptor from the epoll instance
 * 
 * @param poll_fd File descriptor for the epoll instance
 * @param fd File descriptor to be removed
 * @param events Events to be removed
 * @param poll_events Event queue from the server
 * @return 0 on success, -1 on error
 */
int poll_del(int poll_fd, int fd, int events, struct epoll_event *poll_events);

/**
 * @brief Modify a file descriptor's state in the epoll instance
 * 
 * @param poll_fd File descriptor for the epoll instance
 * @param fd File descriptor to be modified
 * @param events Events to be modified
 * @param poll_events Event queue from the server
 * @return 0 on success, -1 on error
 */
int poll_mod(int poll_fd, int fd, int events, struct epoll_event *poll_events);

/**
 * @brief Wait for events to occur on an epoll instance
 * 
 * @param poll_fd File descriptor for the epoll instance
 * @param timeout Timeout in milliseconds
 * @param events Event queue from the server
 * @return 0 on success, -1 on error
 */
int poll_wait(int poll_fd, int timeout, struct epoll_event *events);


#endif // _REACTOR_POLL_H
