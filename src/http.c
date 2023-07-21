#include "http.h"

int
http_request(int fd, struct request *req)
{
    ssize_t nread;

    for (;;)
    {
        nread = read_line(fd, req->raw + req->len, BUFSIZ, MSG_DONTWAIT);

        if (errno == EAGAIN)
            return HTTP_READ_AGAIN;

        if (nread == -1)
            return HTTP_ERROR;

        req->len += (size_t)nread;
        ((char *)req->raw)[req->len] = '\n';
        req->len += 1;

        if (nread == 0)
            break;
    }

    return HTTP_SUCCESS;
}
