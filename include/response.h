/**
 * @file response.h
 * @author Richard Nguyen (richard@richardhnguyen.com)
 * @brief HTTP Response handlers for the server header file
 * @version 0.1
 * @date 2023-07-19
 *
 * @copyright Copyright (c) 2023
 */

#ifndef _REACTOR_RESPONSE_H_
#define _REACTOR_RESPONSE_H_ 1

#include "httpdef.h"

struct response
{
    int status;
    int method;
    int content_type;

    char *body;
    size_t body_len;

    struct dict *headers;
    struct dict *accepts;

    int __chunked_state;
    size_t __chunked_offset;
    size_t __chunked_size;
    size_t __chunked_sent;

#define CHUNKSIZE 10240 // 10KB (original chunk size)
#define CHUNKHDR  64
    char __chunked_buf[CHUNKSIZE + CHUNKHDR];

    pthread_rwlock_t rwlock;
};

struct response_json
{
    const char *const key;
    void *value;
    json_type type;
};

struct response *
response_new();

void
response_status(struct response *res, int status);

void
response_method(struct response *res, int method);

int
response_accept(struct response *response, const char *type);

void
response_construct(struct response *res, int status, int method,
                   const char *filename);

void
response_text(struct response *res, const char *str, const size_t len);

void
response_json(struct response *res, const char *str);

void
response_send_file(struct response *res, const char *filename);

void
response_send_bad_request(struct response *res);

void
response_not_found(struct response *res, const char *path);

void
response_send_method_not_allowed(struct response *res, const int method,
                                 const char *path);

void
response_send_internal_server_error(struct response *res);

int
response_send_chunked(struct response *res, int fd);

ssize_t
response_send(struct response *response, int fd);

void
response_free(struct response *response);

#endif // _REACTOR_RESPONSE_H_
