/**
 * @file util.h
 * @author Richard Nguyen (richard@richardhnguyen.com)
 * @brief Utility functions header
 * @version 0.1.1
 * @date 2023-07-14
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef _REACTOR_UTIL_H
#define _REACTOR_UTIL_H 1

#include "defs.h"

/**
 * @brief Read N bytes from FD into BUF or until a newline is read.
 * 
 * @param fd File descriptor to read from
 * @param buf Buffer to store the read data
 * @param n Number of bytes to read
 * 
 * @return Number of bytes read. -1 is returned if an error occurs.
 */
ssize_t read_line(int fd, char *buf, size_t n);

#endif // _REACTOR_UTIL_H
