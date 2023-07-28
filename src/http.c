#include "http.h"

const struct route supported_uris[] = {
    {"/",      "index.html", (HTTP_METHOD_GET | HTTP_METHOD_HEAD), 0},
    {"/about", "about.html", (HTTP_METHOD_GET),                    0},
    {"/form",  "form.html",  (HTTP_METHOD_GET | HTTP_METHOD_POST), 0},
    {NULL,     NULL,         -1,                                   0}
};

static int
_check_with_static_folder(const char *path)
{
    // Prevent malicious path
    if (strcmp(path, ".") == 0 || strcmp(path, "..") == 0)
        return HTTP_BAD_REQUEST;

    struct stat st;

    // Check if a file is found
    if (stat(path, &st) == -1)
        return HTTP_NOT_FOUND;

    // Check if the path is a regular file
    if (!S_ISREG(st.st_mode))
        return HTTP_FORBIDDEN; // This could Not Found to hide 403 in practice

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

static void
_parse_accept_header(const char *string, struct dict *headers)
{
    char *dup, *save_ptr1, *save_ptr2, *token, *key, *value;

    debug("=== Parse Accept header ===\n");

    dup = strdup(string);
    if (dup == NULL)
        DIE("(parse_accept_header) strdup");

    for (token = strtok_r(dup, ",", &save_ptr1); token != NULL;
         token = strtok_r(NULL, ",", &save_ptr1))
    {
        key = token;

        value = strtok_r(key, ";", &save_ptr2);
        value = strtok_r(NULL, ";", &save_ptr2);

        if (dict_put(headers, key, value == NULL ? "" : value) == SUCCESS)
            debug("Accept[%s] = %s\n", key, value);
    }

    debug("=== End of Accept header ===\n\n");

    free(dup);
}

struct route
http_get_uri_handle(const char *path)
{
    if (path == NULL)
        return (struct route){NULL, NULL, -1};

    const struct route *uri;
    int status = 0;

    // Check from the predefined routes to support different HTTP methods
    for (uri = supported_uris; uri->uri != NULL; uri++)
        if (strcmp(path, uri->uri) == 0)
            return *uri;

    // Check if the path is a static file (only GET method allowed)
    status = _check_with_static_folder(path);

    if (status == SUCCESS)
        return (struct route){path, path, (HTTP_METHOD_GET), HTTP_SUCCESS};

    return (struct route){NULL, NULL, -1, status};
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

struct dict *
http_require_accept(struct dict *headers)
{
    struct dict *accept_store = NULL;
    char *content_type        = NULL;
    const char *accept_value  = dict_get(headers, "Accept");

    accept_store = dict_new(NULL, NULL);

    if (accept_store == NULL)
        DIE("(http_require_accept) dict_new");

    // If accept header is not present, assume the user agent accepts all media
    // types. This is the default behavior.
    if (accept_value == NULL)
    {
        dict_put(accept_store, "*/*", "");
        printf("Accept[*/*] = \n");
        return accept_store;
    }

    _parse_accept_header(accept_value, accept_store);

    return accept_store;
}
