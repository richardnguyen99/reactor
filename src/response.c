#include "response.h"

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

    res->status      = -1;
    res->status_text = NULL;
    res->body        = NULL;
    res->body_len    = 0;

    return res;
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

    if (response->status_text != NULL)
        free(response->status_text);

    if (response->headers != NULL)
        dict_delete(response->headers);

    if (response->version != NULL)
        free(response->version);

    free(response);
}
