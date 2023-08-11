// Private functions for request.h

#include "request.h"

int
__set_method(char *method)
{
    if (strcmp(method, "GET") == 0)
        return HTTP_METHOD_GET;
    else if (strcmp(method, "POST") == 0)
        return HTTP_METHOD_POST;
    else if (strcmp(method, "PUT") == 0)
        return HTTP_METHOD_PUT;
    else if (strcmp(method, "DELETE") == 0)
        return HTTP_METHOD_DELETE;
    else if (strcmp(method, "HEAD") == 0)
        return HTTP_METHOD_HEAD;
    else
        return HTTP_METHOD_INVALID;
}

char *
__set_path(char *path)
{
    const size_t len = strlen(path);

    char *_path = (char *)malloc(sizeof(char) * len + 1);

    if (_path == NULL)
        return NULL;

    strcpy(_path, path);

    return _path;
}

char *
__set_version(char *version)
{
    if (strcmp(version, "HTTP/1.1") == 0)
        return strdup(version);
    else
        return NULL;
}

int
_get_start_line(struct request *req, char *buf)
{
    if (buf == NULL)
        return HTTP_BAD_REQUEST;

    char *token;

    token = strtok(buf, " ");

    if (token == NULL)
        return HTTP_BAD_REQUEST;

    req->method = __set_method(token);
    if (req->method == HTTP_METHOD_INVALID)
        return HTTP_BAD_REQUEST;

    token = strtok(NULL, " ");
    if (token == NULL)
        return HTTP_BAD_REQUEST;

    req->path = __set_path(token);
    if (req->path == NULL)
        return HTTP_INTERNAL_SERVER_ERROR;

    token = strtok(NULL, " ");
    if (token == NULL)
        return HTTP_BAD_REQUEST;

    req->version = __set_version(token);
    if (req->version == NULL)
        return HTTP_INTERNAL_SERVER_ERROR;

    return HTTP_SUCCESS;
}

int
_get_header(struct dict *header, const char *buf, const size_t len)
{
    if (buf == NULL)
        return HTTP_BAD_REQUEST;

    char *token, *key_start, *key_end, *value_start, *value_end;

    //        key   value = token + 2
    //        |     |
    // buf -> Host: localhost:8080\r\n <- \0
    //            |
    //            token

    token = strcasestr(buf, ": ");
    if (token == NULL)
        return HTTP_BAD_REQUEST;

    key_start   = (char *)buf;
    key_end     = token;
    value_start = token + 2;
    value_end   = (char *)buf + len;

    char key[key_end - key_start + 1];
    char value[value_end - value_start + 1];

    memcpy(key, key_start, key_end - key_start);
    memcpy(value, value_start, value_end - value_start);

    key[key_end - key_start]       = '\0';
    value[value_end - value_start] = '\0';

    if (dict_put(header, key, value) == ERROR)
        return HTTP_INTERNAL_SERVER_ERROR;

    debug("headers[%s] = %s\n", key, value);

    return HTTP_SUCCESS;
}
