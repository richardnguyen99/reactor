#include "response.h"

struct response *
http_response_new()
{
    struct response *resp = malloc(sizeof(struct response));

    if (resp == NULL)
        return NULL;

    resp->status = -1;

    memset(resp->version, 0, sizeof(resp->version));
    memset(resp->status_text, 0, sizeof(resp->status_text));

    // Let the caller set the body
    resp->body    = NULL;
    resp->headers = dict_new(NULL, NULL);

    return resp;
}

char *
response_compose(struct response *response)
{
    char *msg = NULL;

    return msg;
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

    free(response);
}
