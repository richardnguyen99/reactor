/**
 * @file rx_alloc.c
 * @author Richard Nguyen (richard@richardhnguyen.com)
 * @brief Implementation for memory allocation module
 * @version 0.1
 * @date 2023-08-12
 *
 * @copyright Copyright (c) 2023
 */

#include <rx_defs.h>
#include <rx_core.h>

void *
rx_alloc(size_t size)
{
    void *ptr;

    ptr = malloc(size);

    if (ptr == NULL)
    {
        fprintf(stderr, "\
reactor: [error] rx_alloc: malloc(%ld) failed\n\
",
                size);
    }

    memset(ptr, 0, size);

    fprintf(stdout, "\
rx_alloc: malloc(%ld) = %p\n\
",
            size, ptr);

    return ptr;
}

void *
rx_calloc(size_t nmemb, size_t size)
{
    void *ptr;

    ptr = calloc(nmemb, size);

    if (ptr == NULL)
    {
        fprintf(stderr, "\
reactor: [error] rx_calloc: calloc(%ld, %ld) failed\n\
",
                nmemb, size);
    }

    fprintf(stdout, "\
rx_calloc: calloc(%ld, %ld) = %p\n\
",
            nmemb, size, ptr);

    return ptr;
}
