#include "request.h"

int
_get_start_line(struct request *req, char *buf);

int
_get_header(struct dict *headers, char *buf);

// =============================================================================

struct request *
request_new()
{
    struct request *request = (struct request *)malloc(sizeof(struct request));

    if (request == NULL)
        return NULL;

    request->version = NULL;
    request->path    = NULL;
    request->method  = -1;

    request->raw = (char *)malloc(sizeof(char) * BUFSIZ);
    request->len = 0;
    request->cap = BUFSIZ;

    if (request->raw == NULL)
    {
        free(request);
        return NULL;
    }

    request->headers = dict_new(NULL, NULL);

    if (request->headers == NULL)
    {
        free(request->raw);
        free(request);
        return NULL;
    }

    return request;
}

int
request_parse(struct request *req, int fd)
{
    int status;
    ssize_t nread;
    char buf[BUFSIZ];

    memset(buf, '\0', BUFSIZ);

    /// An HTTP request has the minimum layout as beflow
    ///
    /// GET /some-path HTTP/1.1\r\n     <- Start line       (required)
    /// Host: localhost:8080\r\n        <- Host header      (required)
    /// User-Agent: curl/7.64.1\r\n     <- Header           (optional)
    /// Accept: */*\r\n                 <- Header           (optional)
    /// ...
    ///\r\n                             <- End of headers   (required)
    /// (data)                          <- Body             (optional)
    ///

    nread = read_line(fd, buf, BUFSIZ, MSG_DONTWAIT);

    if (errno == EAGAIN)
        return HTTP_READ_AGAIN;

    status = _get_start_line(req, buf);

    if (status != HTTP_SUCCESS)
        return status;

    for (;;)
    {
        nread = read_line(fd, buf, BUFSIZ, MSG_DONTWAIT);

        if (errno == EAGAIN)
            return HTTP_READ_AGAIN;

        if (nread == 0)
            break;

        if (nread == -1)
            return HTTP_ERROR;

        if (strcmp(buf, "\r\n") == 0)
            break;

        if (_get_header(req->headers, buf) == ERROR)
            return HTTP_ERROR;
    }

    return HTTP_SUCCESS;
}

void
request_free(struct request *request)
{
    if (request->path != NULL)
        free(request->path);

    if (request->version != NULL)
        free(request->version);

    free(request->raw);
    free(request);
}
