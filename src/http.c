#include "http.h"

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
    const char *accept_value =
        headers != NULL ? dict_get(headers, "Accept") : NULL;

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

struct http_obj *
http_new()
{
    struct http_obj *http = NULL;
    http                  = (struct http_obj *)malloc(sizeof(struct http_obj));

    if (http == NULL)
        return NULL;

    http->req = NULL;
    http->res = NULL;

    return http;
}

int
http_request_line(struct http_obj *http)
{
    ssize_t nread;
    const size_t len = BUFSIZ + 1;
    char buf[len];
    int status;

    nread = read_line(http->cfd, buf, len, MSG_DONTWAIT);

    if (nread == -1 && errno == EAGAIN)
        return HTTP_READ_AGAIN;

    if (nread == -1)
        return HTTP_INTERNAL_SERVER_ERROR;

    if (nread == 0)
        return 0;

    // Parse the request line
    status = request_line(http->req, buf, len);

    debug("%s %s %s\n", GET_HTTP_METHOD(http->req->method), http->req->path,
          http->req->version);

    return status;
}

int
http_request_headers(struct http_obj *http)
{
    ssize_t nread;
    const size_t len = BUFSIZ + 1;
    char buf[len];
    int status;

    for (;;)
    {
        nread = read_line(http->cfd, buf, BUFSIZ, MSG_DONTWAIT);

        if (nread == -1 && errno == EAGAIN)
            return HTTP_READ_AGAIN;

        if (nread == -1)
            return HTTP_INTERNAL_SERVER_ERROR;

        if (nread == 0)
            break;

        if (request_header(http->req, buf, len) == ERROR)
            return HTTP_BAD_REQUEST;
    }
    debug("\n");

    return HTTP_SUCCESS;
}

void
http_response_status(struct http_obj *http, int status)
{
    http->res->status = status;
}

void
http_response_send(struct http_obj *http)
{
    struct request *req  = http->req;
    struct response *res = http->res;

    if (res->accepts == NULL)
        res->accepts = http_require_accept(req->headers);

    switch (res->status)
    {
    case HTTP_NOT_FOUND:
        response_not_found(res, req->path);
        break;

    case HTTP_METHOD_NOT_ALLOWED:
        response_send_method_not_allowed(res, req->method, req->path);
        break;

    case HTTP_INTERNAL_SERVER_ERROR:
        response_send_internal_server_error(res);
        break;

    case HTTP_SUCCESS:
    default:
        break;
    }

    if (res->body_len > CHUNKHDR)
    {
        response_send_chunked(res, http->cfd);
        return;
    }

    response_send(res, http->cfd);
}

void
http_free(struct http_obj *http)
{
    if (http == NULL)
        return;

    if (http->req != NULL)
        request_free(http->req);

    if (http->res != NULL)
        response_free(http->res);

    free(http);
}
