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

struct config {
    char root[8192];
    char port[6];
    size_t nthreads;
};

typedef struct config config_t;

int read_config(const char *filename, config_t *conf);


#endif // _REACTOR_UTIL_H
