#include "http.h"

const struct route supported_uris[] = {
    {"/",      "index.html", (HTTP_METHOD_GET | HTTP_METHOD_HEAD)},
    {"/about", "about.html", (HTTP_METHOD_GET)                   },
    {"/form",  "form.html",  (HTTP_METHOD_GET | HTTP_METHOD_POST)},
    {NULL,     NULL,         -1                                  }
};

static int
_check_with_static_folder(const char *path)
{
    // This doesn't actually open the file, it just checks if it exists.
    int status = access(path, F_OK);

    if (status == -1)
        return ERROR;

    return SUCCESS;
}

static int
_default_method_handler(const char *value)
{
    if (value == NULL)
        return ERROR;

    if (strlen(value) == 0)
        return ERROR;

    return SUCCESS;
}

struct route
http_get_uri_handle(const char *path)
{
    if (path == NULL)
        return (struct route){NULL, NULL, -1};

    const struct route *uri;

    // Check from the predefined routes to support different HTTP methods
    for (uri = supported_uris; uri->uri != NULL; uri++)
    {
        if (strcmp(path, uri->uri) == 0)
            return *uri;
    }

    // Check if the path is a static file (only GET method allowed)
    if (_check_with_static_folder(path) == SUCCESS)
        return (struct route){path, path, (HTTP_METHOD_GET)};

    return (struct route){NULL, NULL, -1};
}

char *
http_get_status_text(int status)
{
    char *status_text = NULL;

    // Generate the status text based on the status code please
    const size_t len = strlen(GET_HTTP_MSG(status));

    status_text = (char *)malloc(len + 1);
    if (status_text == NULL)
        return NULL;

    strncpy(status_text, GET_HTTP_MSG(status), len);

    status_text[len] = '\0';

    return status_text;
}

int
http_require_header(struct dict *headers, const char *key,
                    http_header_handler func)
{
    if (headers == NULL || key == NULL)
        return ERROR;

    // Check if the header is present
    char *value = dict_get(headers, key);
    if (value == NULL)
        return ERROR;

    if (_default_method_handler(value) == ERROR)
        return ERROR;

    if (func != NULL && func(value) == ERROR)
        return ERROR;

    return SUCCESS;
}
