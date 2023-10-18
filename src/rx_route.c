/* MIT License
 *
 * Copyright (c) 2023 Richard H. Nguyen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <rx_config.h>
#include <rx_core.h>

/* clang-format off */
const struct rx_route router_table[] = {
{
    .endpoint = "/",
    .resource = "pages/index.html",
    .handler  = {
        .get    = rx_route_index_get,
        .post   = NULL,
        .put    = NULL,
        .patch  = NULL,
        .delete = NULL,
        .head   = NULL
    }
},
{
    .endpoint = "/login",
    .resource = "pages/login.html",
    .handler  = {
        .get    = rx_route_login_get,
        .post   = rx_route_login_post,
        .put    = NULL,
        .patch  = NULL,
        .delete = NULL,
        .head   = NULL
    }

},
{
    .endpoint = "/about",
    .resource = "pages/about.html",
    .handler  = {
        .get    = rx_route_about_get,
        .post   = NULL,
        .put    = NULL,
        .patch  = NULL,
        .head   = NULL
    }
},
{
    .endpoint = NULL,
    .resource = NULL,
    .handler  = {
        .get    = NULL,
        .post   = NULL,
        .put    = NULL,
        .patch  = NULL,
        .delete = NULL,
        .head   = NULL
    },
}};
/* clang-format on */

int
rx_route_get(struct rx_route *storage, const char *endpoint, size_t ep_len)
{
    int i;

    if (endpoint == NULL || storage == NULL || ep_len == 0)
        return RX_ERROR;

    for (i = 0; router_table[i].endpoint != NULL; i++)
    {
        if (strncasecmp(router_table[i].endpoint, endpoint, ep_len) == 0)
        {
            storage->endpoint = router_table[i].endpoint;
            storage->resource = router_table[i].resource;
            storage->handler  = router_table[i].handler;

            return RX_OK;
        }
    }

    if (ep_len > 8 && strncasecmp(endpoint, "/public/", 8) == 0)
    {
        storage->endpoint = "/public/";
        storage->resource = endpoint;

        memset(&storage->handler, 0, sizeof(struct rx_router_handler));

        storage->handler.get    = rx_route_static;
        storage->handler.post   = NULL;
        storage->handler.put    = NULL;
        storage->handler.patch  = NULL;
        storage->handler.delete = NULL;
        storage->handler.head   = NULL;

        return RX_OK;
    }

    return RX_ERROR;
}

void *
rx_route_index_get(struct rx_request *req, struct rx_response *res)
{
    NOOP(req);

    rx_response_render(res, "pages/index.html");

    return NULL;
}

void *
rx_route_login_get(struct rx_request *req, struct rx_response *res)
{
    NOOP(req);

    rx_response_render(res, "pages/login.html");

    return NULL;
}

void *
rx_route_login_post(struct rx_request *req, struct rx_response *res)
{
    NOOP(req);

    const char *body = "{\"data\": \"Hello, World!\", \"status\": 200}";

    res->content_type = RX_HTTP_MIME_APPLICATION_JSON;

    rx_response_send(res, body, strlen(body));

    return NULL;
}

void *
rx_route_about_get(struct rx_request *req, struct rx_response *res)
{
    NOOP(req);

    rx_response_render(res, "pages/about.html");

    return NULL;
}

void *
rx_route_static(struct rx_request *req, struct rx_response *res)
{
    const size_t resource_len = req->uri.path_end - req->uri.path - 1;

    int ret;
    struct rx_file file;
    char resource[resource_len], *buf;

    memset(&file, 0, sizeof(file));
    memcpy(resource, req->uri.path + 1, req->uri.path_end - req->uri.path);
    resource[resource_len] = '\0';

    rx_log(LOG_LEVEL_0, LOG_TYPE_INFO,
           "[Thread %ld]%4.sStatic file request: %s\n", pthread_self(), "",
           resource);

    ret = rx_file_open(&file, resource, O_RDONLY);

    if (ret == RX_FATAL_WITH_ERROR)
    {
        rx_log(LOG_LEVEL_0, LOG_TYPE_ERROR, "Failed to open file\n");
        return RX_ERROR_PTR;
    }

    buf = (char *)mmap(NULL, file.size, PROT_READ, MAP_PRIVATE, file.fd, 0);

    if (buf == MAP_FAILED)
    {
        rx_log(LOG_LEVEL_0, LOG_TYPE_ERROR, "mmap: %s\n", strerror(errno));
        return RX_ERROR_PTR;
    }

    res->is_content_mmapd = 1;
    res->content          = buf;
    res->content_length   = file.size;
    res->content_type     = file.mime;
    res->status_code      = RX_HTTP_STATUS_CODE_OK;
    res->status_message =
        (char *)rx_response_status_message(RX_HTTP_STATUS_CODE_OK);

    rx_file_close(&file);

    return RX_OK_PTR;
}

void *
rx_route_4xx(struct rx_request *req, struct rx_response *res, int code)
{
#ifdef RX_DEBUG
    rx_log(LOG_LEVEL_0, LOG_TYPE_WARN,
           "[Thread %ld]%4.sClient error with code %d\n", pthread_self(), "",
           code);
#endif
    char msg[80], reason[3000], *buf;
    int buflen;

    switch (code)
    {
    case RX_HTTP_STATUS_CODE_NOT_FOUND:
        sprintf(msg, "Not Found");
        sprintf(reason, "The requested resource (%s) could not be found.",
                req->uri.raw_uri);

        break;

    case RX_HTTP_STATUS_CODE_METHOD_NOT_ALLOWED:
        sprintf(msg, "Method Not Allowed");
        sprintf(reason,
                "The requested resource (%s) does not support the "
                "method %s.",
                req->uri.raw_uri, rx_request_method_str(req->method));

        break;

    case RX_HTTP_STATUS_CODE_BAD_REQUEST:
    default:
        sprintf(msg, "Bad Request");
        sprintf(reason, "The server could not process this request due to "
                        "malformed request.");
        break;
    }

    buflen = asprintf(&buf, rx_view_engine.client_error_template.data, code,
                      msg, reason);

    if (buflen == RX_ERROR)
    {
        rx_log(LOG_LEVEL_0, LOG_TYPE_ERROR, "asprintf: %s\n", strerror(errno));
        return RX_ERROR_PTR;
    }

    buflen = asprintf(&res->content, rx_view_engine.base_template.data, buf);

    if (buflen == RX_ERROR)
    {
        rx_log(LOG_LEVEL_0, LOG_TYPE_ERROR, "asprintf: %s\n", strerror(errno));
        return RX_ERROR_PTR;
    }

    res->content_length = (size_t)buflen;
    res->content_type   = RX_HTTP_MIME_TEXT_HTML;
    res->status_code    = code;
    res->status_message = (char *)rx_response_status_message(code);

    free(buf);

    return NULL;
}
