/**
 * @file util.h
 * @author Richard Nguyen (richard@richardhnguyen.com)
 * @brief Utility functions header
 * @version 0.1
 * @date 2023-06-30
 *
 * @copyright Copyright (c) 2023
 */

#ifndef _JAWS_UTIL_H
#define _JAWS_UTIL_H 1

#include "defs.h"

#define JENV_RDOK 0        /* Read env OK */
#define JENV_RDERROR (-1)  /* Read env ERROR */
#define JENV_ROOT_ERR (-2) /* Read invalid root dir */
#define JENV_PORT_ERR (-3) /* Read invalid port number */
#define JENV_NTHR_ERR (-4) /* Read invalid thread number */

struct keypair
{
    char key[120];
    char value[8190];
};

typedef struct keypair keypair_t;

int readenv(const char *filename, struct configopt *conf);

void lower(char *str);
void upper(char *str);

keypair_t getkeypair(const char *str, const size_t maxlen, const char *delim);

int buildfilepath(const char *filename, char *result, size_t *len);
int checkfile(const char *filename);

#endif // _JAWS_UTIL_H
