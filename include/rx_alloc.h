/**
 * @file rx_alloc.h
 * @author Richard Nguyen (richard@richardhnguyen.com)
 * @brief Header for safe and convenient memory allocation.
 * @version 0.1
 * @date 2023-08-12
 *
 * @copyright Copyright (c) 2023
 */

#ifndef _RX_ALLOC_H_
#define _RX_ALLOC_H_ 1

#include <rx_defs.h>
#include <rx_core.h>

void *
rx_alloc(const size_t size);

void *
rx_calloc(const size_t count, const size_t size);

#if defined(RX_HAS_POSIX_MEMALIGN)

void *
rx_memalign(const size_t alignment, const size_t size);

#endif /* RX_HAVE_POSIX_MEMALIGN */

#endif /* _RX_ALLOC_H_ */
