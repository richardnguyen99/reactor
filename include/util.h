/**
 * @file util.h
 * @author Richard Nguyen (richard@richardhnguyen.com)
 * @brief Utility functions header file.
 * @version 0.1
 * @date 2023-07-18
 *
 * @copyright Copyright (c) 2023
 */

#ifndef _REACTOR_UTILITY_H_
#define _REACTOR_UTILITY_H_ 1

#include "defs.h"

ssize_t
read_until(int fd, void *buf, size_t buflen, const char *delim, int flags);

ssize_t
read_line(int fd, char *buf, size_t buflen, int flags);

#endif // _REACTOR_UTILITY_H_
