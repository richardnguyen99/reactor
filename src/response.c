#include "response.h"

size_t count = 0;

char *
__get_internal_server(size_t *len)
{
    char *buf = NULL;
    struct stat st;
    int ffd;

    if ((ffd = open("500.html", O_RDONLY, 0)) == -1)
        DIE("(handle_request) open");

    if (fstat(ffd, &st) == -1)
        DIE("(handle_request) fstat");

    buf = (char *)mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, ffd, 0);
    if (buf == NULL)
        DIE("(response_construct) mmap");

    *len = st.st_size;

    return buf;
}

char *
__get_body(const char *filename, size_t *len, int *status)
{
    int ffd;
    struct stat st;
    char *buf;

    if ((ffd = open(filename, O_RDONLY, 0)) == -1)
    {
        *status = HTTP_INTERNAL_SERVER_ERROR;
        buf     = __get_internal_server(len);
        return buf;
    }

    if (fstat(ffd, &st) == -1)
    {
        *status = HTTP_INTERNAL_SERVER_ERROR;
        buf     = __get_internal_server(len);
        return buf;
    }

    buf = (char *)mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, ffd, 0);
    if (buf == MAP_FAILED)
        DIE("(response_construct) mmap");

    *len = st.st_size;

    close(ffd);

    return buf;
}

json_object *
__set_json(json_type type, void *value, const size_t len)
{
    json_object *obj = NULL;

    switch (type)
    {
    case json_type_boolean:
        obj = json_object_new_boolean(*(bool *)value);
        break;
    case json_type_double:
        obj = json_object_new_double(*(double *)value);
        break;
    case json_type_int:
        obj = json_object_new_int(*(int *)value);
        break;
    case json_type_string:
        obj = json_object_new_string((char *)value);
        break;
    case json_type_array:
        obj = json_object_new_array_ext(len);

        for (size_t i = 0; i < len; ++i)
            json_object_array_put_idx(obj, i, __set_json(type, value, len));

        break;
    default:
        break;
    }

    return obj;
}

struct response *
response_new()
{
    struct response *res = (struct response *)malloc(sizeof(struct response));

    if (res == NULL)
        DIE("(response_new) malloc");

    res->headers = dict_new(NULL, NULL);
    if (res->headers == NULL)
        DIE("(response_new) dict_new");

    res->accepts = NULL;

    /* Set these fields later. Otherwise, they will yield an error */

    res->status       = -1;
    res->method       = -1;
    res->content_type = HTTP_CONTENT_TYPE_INVALID;

    res->__chunked_state  = 0;
    res->__chunked_offset = 0;
    res->__chunked_size   = 0;

    res->body     = NULL;
    res->body_len = 0;

    pthread_rwlock_init(&res->rwlock, NULL);

    return res;
}

void
response_status(struct response *res, int status)
{
    res->status = status;
}

void
response_method(struct response *res, int method)
{
    res->method = method;
}

int
response_accept(struct response *res, const char *type)
{
    bool accept_all = false;

    if (res == NULL)
        return HTTP_CONTENT_TYPE_INVALID;

    if (dict_get(res->accepts, "*/*") != NULL)
        accept_all = true;

    if (strcmp(type, "html") == 0 &&
        (accept_all || dict_get(res->accepts, "text/html") != NULL))
        return HTTP_CONTENT_TYPE_HTML;

    if (strcmp(type, "css") == 0 &&
        (accept_all || dict_get(res->accepts, "text/css") != NULL))
        return HTTP_CONTENT_TYPE_CSS;

    if (strcmp(type, "js") == 0 &&
        (accept_all ||
         dict_get(res->accepts, "application/javascript") != NULL))
        return HTTP_CONTENT_TYPE_JS;

    if (strcmp(type, "png") == 0 &&
        (accept_all || dict_get(res->accepts, "image/png") != NULL))
        return HTTP_CONTENT_TYPE_PNG;

    if (strcmp(type, "jpg") == 0 &&
        (accept_all || dict_get(res->accepts, "image/jpeg") != NULL))
        return HTTP_CONTENT_TYPE_JPEG;

    if (strcmp(type, "webp") == 0 &&
        (accept_all || dict_get(res->accepts, "image/webp") != NULL))
        return HTTP_CONTENT_TYPE_WEBP;

    if (strcmp(type, "svg") == 0 &&
        (accept_all || dict_get(res->accepts, "image/svg+xml") != NULL))
        return HTTP_CONTENT_TYPE_SVG;

    if (strcmp(type, "ico") == 0 &&
        (accept_all || dict_get(res->accepts, "image/x-icon") != NULL ||
         dict_get(res->accepts, "image/avif") != NULL))
        return HTTP_CONTENT_TYPE_ICON;

    if (strcmp(type, "txt") == 0 &&
        (accept_all || dict_get(res->accepts, "text/plain") != NULL))
        return HTTP_CONTENT_TYPE_TEXT;

    if (strcmp(type, "json") == 0 &&
        (accept_all || dict_get(res->accepts, "application/json")))
        return HTTP_CONTENT_TYPE_JSON;

    return accept_all ? HTTP_CONTENT_TYPE_HTML : HTTP_CONTENT_TYPE_INVALID;
}

void
response_send_file(struct response *res, const char *filename)
{
    res->body = __get_body(filename, &res->body_len, &res->status);
    if (res->body == NULL)
        DIE("(response_send_file) __get_body");
}

void
response_construct(struct response *res, int status, int method,
                   const char *filename)
{
    int content_type;
    const char *ext = strrchr(filename, '.');

    res->status = status;
    res->method = method;

    content_type = response_accept(res, ext + 1);

    if (content_type == HTTP_CONTENT_TYPE_INVALID)
    {
        res->status = HTTP_NOT_ACCEPTABLE;
        response_text(res, "Not acceptable", 14);

        return;
    }

    res->content_type = content_type;
    res->body         = __get_body(filename, &res->body_len, &res->status);
    if (res->body == NULL)
        DIE("(response_construct) mmap");
}

void
response_text(struct response *res, const char *str, const size_t len)
{
    res->body = strdup(str);

    if (res->body == NULL)
        DIE("(response_text) strdup");

    res->body_len     = len;
    res->content_type = HTTP_CONTENT_TYPE_TEXT;
}

void
response_json(struct response *res, const char *str)
{
    printf("Create json object\n");
    json_object *obj = json_tokener_parse(str);
    if (obj == NULL)
        DIE("(response_json) json_tokener_parse");

    char *buf = (char *)json_object_to_json_string_ext(
        obj, JSON_C_TO_STRING_PLAIN | JSON_C_TO_STRING_NOSLASHESCAPE);

    if (buf == NULL)
        DIE("(response_json) json_object_to_json_string_ext");

    res->body = strdup(buf);

    if (res->body == NULL)
        DIE("(response_json) strdup");

    res->body_len = strlen(res->body);

    if (json_object_put(obj) != 1)
        DIE("(response_json) json_object_put");
}

void
response_send_bad_request(struct response *res)
{
    printf("Send bad request\n");
    char buf[BUFSIZ];
    res->content_type = HTTP_CONTENT_TYPE_JSON;
    res->status       = HTTP_BAD_REQUEST;

    // clang-format off

    snprintf(buf, BUFSIZ,
             "{\r\n"
             "\"status\": 400,\r\n"
             "\"message\": \"The server cannot process your request at this"
                            "moment due to an ill-formed request\"\r\n"
             "}\r\n");

    // clang-format on

    response_json(res, buf);
}

void
response_send_not_found(struct response *res, const char *path)
{
    res->status = HTTP_NOT_FOUND;

    int content_type = response_accept(res, "html");
    if (content_type == HTTP_CONTENT_TYPE_ALL ||
        content_type == HTTP_CONTENT_TYPE_HTML)
    {
        res->content_type = content_type;
        response_send_file(res, "404.html");

        return;
    }

    content_type = response_accept(res, "json");
    if (content_type == HTTP_CONTENT_TYPE_JSON)
    {
        char buf[BUFSIZ];
        res->content_type = content_type;

        snprintf(buf, BUFSIZ,
                 "{\r\n"
                 "\"status\": 404,\r\n"
                 "\"message\": \"Resource '%s' not found\"\r\n"
                 "}\r\n",
                 path);

        response_json(res, buf);

        return;
    }

    res->content_type = HTTP_CONTENT_TYPE_TEXT;
    response_text(res, "404 Not found", 14);
}

void
response_send_method_not_allowed(struct response *res, const int method,
                                 const char *path)
{
    char buf[BUFSIZ];
    int content_type = response_accept(res, "json");
    if (content_type == HTTP_CONTENT_TYPE_INVALID)
    {
        res->status = HTTP_NOT_ACCEPTABLE;
        response_text(res, "Not acceptable", 14);

        return;
    }

    res->status       = HTTP_METHOD_NOT_ALLOWED;
    res->method       = method;
    res->content_type = content_type;

    snprintf(buf, BUFSIZ,
             "{\r\n"
             "\"status\": %d,\r\n"
             "\"message\": \"Method %s is not allowed for '%s'\"\r\n"
             "}\r\n",
             HTTP_METHOD_NOT_ALLOWED, GET_HTTP_METHOD(method), path);

    response_json(res, buf);
}

void
response_send_internal_server_error(struct response *res)
{
    res->status = HTTP_NOT_FOUND;

    int content_type = response_accept(res, "html");
    if (content_type == HTTP_CONTENT_TYPE_ALL ||
        content_type == HTTP_CONTENT_TYPE_HTML)
    {
        res->content_type = content_type;
        response_send_file(res, "500.html");

        return;
    }

    content_type = response_accept(res, "json");
    if (content_type == HTTP_CONTENT_TYPE_JSON)
    {
        char buf[BUFSIZ];
        res->content_type = content_type;

        snprintf(
            buf, BUFSIZ,
            "{\r\n"
            "\"status\": 500,\r\n"
            "\"message\": \"It seems that the server has an internal error \
                            that prevents it from processing your request. \
                            Please try again later.\"\r\n"
            "}\r\n");

        response_json(res, buf);

        return;
    }

    res->content_type = HTTP_CONTENT_TYPE_TEXT;
    response_text(res, "500 Internal Server Error", 25);
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

    if (response->body != NULL &&
        response->content_type == HTTP_CONTENT_TYPE_TEXT)
        free(response->body);
    else if (response->body != NULL &&
             response->content_type == HTTP_CONTENT_TYPE_JSON)
    {
        printf("Freeing json object\n");
        free(response->body);
    }
    else if (response->body != NULL)
        munmap(response->body, response->body_len);

    if (response->headers != NULL)
        dict_delete(response->headers);

    if (response->accepts != NULL)
        dict_delete(response->accepts);

    pthread_rwlock_destroy(&response->rwlock);

    free(response);
}
