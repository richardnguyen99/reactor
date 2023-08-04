#include "route.h"

void
_static_get_handler(struct request *req, struct response *res)
{
    const char *ext = strrchr(req->path, '.') + 1;
    int accept_ret;

    // There must be some errors within the server
    if ((accept_ret = response_accept(res, ext)) == HTTP_CONTENT_TYPE_INVALID)
        return;

    if (accept_ret == HTTP_CONTENT_TYPE_INVALID)
        return;

    res->status       = HTTP_SUCCESS;
    res->method       = HTTP_METHOD_GET;
    res->content_type = accept_ret;

    response_send_file(res, req->path + 1);
}

int
_check_with_static_folder(const char *endpoint)
{
    int status       = HTTP_SUCCESS;
    const char *path = endpoint + 1; // Skip the first slash
    struct stat st;

    if (stat(path, &st) == -1)
    {
        status = errno == ENOENT ? HTTP_NOT_FOUND : HTTP_INTERNAL_SERVER_ERROR;
        return status;
    }

    if (!S_ISREG(st.st_mode))
    {
        status = HTTP_FORBIDDEN;
        return status;
    }

    return status;
}

struct __route
route_get_handler(const char *endpoint)
{
    int i, status;

    // Check pre-defined endpoints
    for (i = 0; router_table[i].endpoint != NULL; i++)
    {

        if (strcmp(router_table[i].endpoint, endpoint) == 0)
            return (struct __route){.endpoint = router_table[i].endpoint,
                                    .resource = router_table[i].resource,
                                    .handler  = router_table[i].handler,
                                    .status   = HTTP_SUCCESS};
    }

    status = _check_with_static_folder(endpoint);
    if (status == HTTP_SUCCESS)
        return (struct __route){.endpoint = endpoint,
                                .resource = endpoint,
                                .handler  = {.get = _static_get_handler},
                                .status   = HTTP_SUCCESS};

    return (struct __route){NULL, NULL, {0}, status};
}
