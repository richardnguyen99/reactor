/**
 * @file ring_buffer.h
 * @author Richard Nguyen (richard@richardhnguyen.com)
 * @brief Ring buffer header file
 * @version 0.1
 * @date 2023-07-16
 * 
 * @copyright Copyright (c) 2023
 */

#ifndef _RING_BUFFER_H
#define _RING_BUFFER_H 1

#include "defs.h"

struct queue {
    /* Capacity of file descriptors in this ring buffer */
    size_t cap;

    /* Current number of file descriptors in this ring buffer */
    size_t size;

    /* Index position for enqueueing a new file descriptor */
    size_t in;

    /* Index position for dequeueing a file descriptor */
    size_t out;

    /* List of file descriptors in this ring buffer */
    int *fds;
};

/**
 * @brief Initialize a malloc'd ring buffer to store file descriptors
 * 
 * @param cap Maximum number of file descriptors to store
 * @return Pointer to the ring buffer. NULL if malloc fails.
 */
struct queue *queue_init(size_t cap);

/**
 * @brief Push a file descriptor to the back of the ring buffer
 * 
 * @param q Pointer to the ring buffer
 * @param fd File descriptor to push
 */
void queue_push_back(struct queue *q, int fd);

/**
 * @brief Remove the file descriptor at the front of the ring buffer
 * 
 * @param q Pointer to the ring buffer
 * @return File descriptor at the front of the ring buffer
 */
int queue_pop_front(struct queue *q);

/**
 * @brief Free the ring buffer
 * 
 * @param q Pointer to the ring buffer
 */
void queue_free(struct queue *q);

#endif // _RING_BUFFER_H
