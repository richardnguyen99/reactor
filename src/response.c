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

    close(ffd);

    return buf;
}

struct response *
response_new()
{
    struct response *res = (struct response *)malloc(sizeof(struct response));

    if (res == NULL)
        DIE("(response_new) malloc");

    res->headers = dict_new(NULL, NULL);
    if (res->headers == NULL)
        DIE("(response_new) dict_new");

    res->accepts = NULL;

    /* Set these fields later. Otherwise, they will yield an error */

    res->status       = -1;
    res->method       = -1;
    res->content_type = HTTP_CONTENT_TYPE_INVALID;

    res->body     = NULL;
    res->body_len = 0;

    return res;
}

int
response_accept(struct response *res, const char *type)
{
    bool accept_all = false;

    if (res == NULL)
        return HTTP_CONTENT_TYPE_INVALID;

    if (dict_get(res->accepts, "*/*") != NULL)
        accept_all = true;

    if (strcmp(type, "html") == 0 &&
        (accept_all || (res->accepts, "text/html") != NULL))
        return HTTP_CONTENT_TYPE_HTML;

    if (strcmp(type, "css") == 0 &&
        (accept_all || dict_get(res->accepts, "text/css") != NULL))
        return HTTP_CONTENT_TYPE_CSS;

    if (strcmp(type, "js") == 0 &&
        (accept_all || dict_get(res->accepts, "text/javascript") != NULL))
        return HTTP_CONTENT_TYPE_JS;

    if (strcmp(type, "png") == 0 &&
        (accept_all || dict_get(res->accepts, "image/png") != NULL))
        return HTTP_CONTENT_TYPE_PNG;

    if (strcmp(type, "jpg") == 0 &&
        (accept_all || dict_get(res->accepts, "image/jpeg") != NULL))
        return HTTP_CONTENT_TYPE_JPEG;

    if (strcmp(type, "webp") == 0 &&
        (accept_all || dict_get(res->accepts, "image/webp") != NULL))
        return HTTP_CONTENT_TYPE_WEBP;

    if (strcmp(type, "svg") == 0 &&
        (accept_all || dict_get(res->accepts, "image/svg+xml") != NULL))
        return HTTP_CONTENT_TYPE_SVG;

    if (strcmp(type, "icon") == 0 &&
        (accept_all || dict_get(res->accepts, "image/x-icon") != NULL ||
         dict_get(res->accepts, "image/avif") != NULL))
        return HTTP_CONTENT_TYPE_ICON;

    if (strcmp(type, "txt") == 0 &&
        (accept_all || dict_get(res->accepts, "text/plain") != NULL))
        return HTTP_CONTENT_TYPE_TEXT;

    if (strcmp(type, "json") == 0 &&
        (accept_all || dict_get(res->accepts, "application/json")))
        return HTTP_CONTENT_TYPE_JSON;

    return FAILURE;
}

void
response_construct(struct response *res, int status, int method,
                   const char *filename)
{
    int content_type;
    const char *ext = strrchr(filename, '.');

    res->status = status;
    res->method = method;

    content_type = response_accept(res, ext + 1);

    if (content_type == HTTP_CONTENT_TYPE_INVALID)
    {
        res->body = __get_body("406.html", &res->body_len, &res->status);
        if (res->body == NULL)
            DIE("(response_construct) mmap");

        return;
    }

    res->content_type = content_type;
    res->body         = __get_body(filename, &res->body_len, &res->status);
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

    if (response->accepts != NULL)
        dict_delete(response->accepts);

    free(response);
}
