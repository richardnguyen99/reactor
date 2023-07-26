#include "response.h"

char *
__get_internal_server(size_t *len)
{
    char *buf = NULL;
    struct stat st;
    int ffd;

    if ((ffd = open("500.html", O_RDONLY, 0)) == -1)
        DIE("(handle_request) open");

    if (fstat(ffd, &st) == -1)
        DIE("(handle_request) fstat");

    buf = (char *)mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, ffd, 0);
    if (buf == NULL)
        DIE("(response_construct) mmap");

    *len = st.st_size;

    return buf;
}

char *
__get_body(const char *filename, size_t *len, int *status)
{
    int ffd;
    struct stat st;
    char *buf;

    if ((ffd = open(filename, O_RDONLY, 0)) == -1)
    {
        *status = HTTP_INTERNAL_SERVER_ERROR;
        buf     = __get_internal_server(len);
        return buf;
    }

    if (fstat(ffd, &st) == -1)
    {
        *status = HTTP_INTERNAL_SERVER_ERROR;
        buf     = __get_internal_server(len);
        return buf;
    }

    buf = (char *)mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, ffd, 0);
    if (buf == MAP_FAILED)
        DIE("(response_construct) mmap");

    *len = st.st_size;

    return buf;
}

struct response *
response_new()
{
    struct response *res = (struct response *)malloc(sizeof(struct response));

    if (res == NULL)
        return NULL;

    res->headers = dict_new(NULL, NULL);
    if (res->headers == NULL)
    {
        free(res);
        return NULL;
    }

    res->status   = -1;
    res->method   = -1;
    res->body     = NULL;
    res->body_len = 0;

    return res;
}

void
response_construct(struct response *res, int status, int method,
                   const char *filename)
{
    res->status = status;
    res->method = method;

    res->body = __get_body(filename, &res->body_len, &res->status);
    if (res->body == NULL)
        DIE("(response_construct) mmap");
}

ssize_t
response_send(struct response *response, int fd)
{
    ssize_t nsent = 0;

    return nsent;
}

void
response_free(struct response *response)
{
    if (response == NULL)
        return;

    if (response->body != NULL)
        munmap(response->body, response->body_len);

    if (response->headers != NULL)
        dict_delete(response->headers);

    free(response);
}
