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
    request->method  = HTTP_METHOD_INVALID;
    request->status  = HTTP_NOT_SET;

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
    size_t total;
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

    if (req->method != HTTP_METHOD_INVALID)
        goto read_header;

    debug("\n=== Reading request line: ");
    nread = read_line(fd, buf, BUFSIZ, MSG_DONTWAIT);

    if (nread == -1 && errno == EAGAIN)
        return HTTP_READ_AGAIN;

    if (nread == -1)
        DIE("(request_parse) read_line");

    if (nread == 0)
        return HTTP_READ_AGAIN;

    status = _get_start_line(req, buf);

    if (status == HTTP_BAD_REQUEST || status == HTTP_INTERNAL_SERVER_ERROR)
        DIE("(request_parse) _get_start_line");

    if (status != HTTP_SUCCESS)
        return status;

    req->status = status;

    debug("%s %s %s ===\n", GET_HTTP_METHOD(req->method), req->path,
          req->version);
    debug("\n=== Reading request headers ===\n");

read_header:
    for (;;)
    {
        nread = read_line(fd, buf, BUFSIZ, MSG_DONTWAIT);

        if (nread == -1 && errno == EAGAIN)
            return HTTP_READ_AGAIN;

        if (nread == -1)
            return HTTP_ERROR;

        if (nread == 0)
            break;

        if (_get_header(req->headers, buf) == ERROR)
            return HTTP_ERROR;
    }

    debug("=== End of headers ===\n\n");

    // TODO: Parse body

    return HTTP_SUCCESS;
}

void
request_free(struct request *request)
{
    if (request->path != NULL)
        free(request->path);

    if (request->version != NULL)
        free(request->version);

    if (request->headers != NULL)
        dict_delete(request->headers);

    if (request->raw != NULL)
        free(request->raw);

    free(request);
}
