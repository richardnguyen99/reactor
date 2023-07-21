#include "request.h"

struct request *
request_new()
{
    struct request *request = (struct request *)malloc(sizeof(struct request));

    if (request == NULL)
        return NULL;

    memset(request->path, '\0', BUFSIZ);
    memset(request->version, '\0', HTTP_VER_SIZE);

    request->method = -1;

    request->raw = (char *)malloc(sizeof(char) * BUFSIZ);
    request->len = 0;
    request->cap = BUFSIZ;

    if (request->raw == NULL)
    {
        free(request);
        return NULL;
    }

    return request;
}

void
request_free(struct request *request)
{
    free(request->raw);
    free(request);
}
