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

static void
rx_memset_uri(struct rx_request_uri *uri);

static void
rx_memset_version(struct rx_request_version *version);

static void
rx_memset_header_host(struct rx_header_host *host);

static void
rx_memset_header_accept_encoding(
    struct rx_header_accept_encoding *accept_encoding
);

static void
rx_memset_header_accept(struct rx_qlist *accept);

static void
rx_parse_ae_header(
    struct rx_header_accept_encoding *ae, const char *buffer, size_t len
);

static void
rx_parse_accept_header(struct rx_qlist *accept, const char *buffer, size_t len);

static double
rx_parse_q_value(const char *buffer, size_t len);

static int
rx_request_hdrncmp(const char *str, size_t len, const char *key);

int
rx_request_init(struct rx_request *request)
{
    request->method = RX_REQUEST_METHOD_INVALID;

    memset(&request->uri, 0, sizeof(request->uri));
    memset(&request->version, 0, sizeof(request->version));
    memset(&request->host, 0, sizeof(request->host));
    memset(&request->accept, 0, sizeof(request->accept));

    rx_memset_uri(&request->uri);
    rx_memset_version(&request->version);
    rx_memset_header_host(&request->host);
    rx_memset_header_accept_encoding(&request->accept_encoding);
    rx_memset_header_accept(&request->accept);

    request->state = RX_REQUEST_STATE_READY;

    return RX_OK;
}

void
rx_request_destroy(struct rx_request *request)
{
    rx_qlist_destroy(&request->accept);
}

int
rx_request_process_start_line(
    struct rx_request *request, const char *buffer, size_t len
)
{
#if defined(RX_DEBUG)
    pthread_t tid = pthread_self();

    rx_log(
        LOG_LEVEL_0, LOG_TYPE_DEBUG,
        "[Thread %ld]%4.sProcessing request line\n", tid, ""
    );
#endif
    if (buffer == NULL || len == 0)
    {
        return RX_ERROR;
    }

    int ret;
    const char *begin, *end;
    size_t i;

    i     = 0;
    begin = end = buffer;
    ret         = RX_OK;

    for (; i < len && buffer[i] != ' ' && buffer[i] != '\0'; ++i)
    {
        end = end + 1;
    }

    if (i == len || buffer[i] == '\0')
    {
        request->state = RX_REQUEST_STATE_DONE;
        return RX_ERROR;
    }

    ret = rx_request_proccess_method(&request->method, begin, end - begin);

    if (ret != RX_OK)
    {
        return RX_ERROR;
    }

    begin = end + 1;
    end   = end + 1;
    i     = i + 1;

    for (; i < len && buffer[i] != ' ' && buffer[i] != '\0'; ++i)
    {
        end = end + 1;
    }

    if (i == len || buffer[i] == '\0')
    {
        request->state = RX_REQUEST_STATE_DONE;
        return RX_ERROR;
    }

    ret = rx_request_process_uri(&request->uri, begin, end - begin);

    if (ret != RX_OK)
    {
        return RX_ERROR;
    }

    begin = end + 1;
    end   = buffer + len;

    ret = rx_request_process_version(&request->version, begin, end - begin);

    return RX_OK;
}

int
rx_request_process_headers(
    struct rx_request *request, const char *buffer, size_t len
)
{
    int ret;
    const char *key_begin, *key_end, *value_begin, *value_end;

    ret       = RX_OK;
    key_begin = key_end = value_begin = value_end = buffer;

    if (buffer == NULL || len == 0)
    {
        ret = RX_ERROR;
        goto end;
    }

    value_end = strstr(key_begin, "\r\n");

    while (value_end != NULL && value_end - key_begin > 0)
    {

        ret = rx_request_parse_header(
            key_begin, value_end - key_begin, &key_begin, &key_end,
            &value_begin, &value_end
        );

        if (ret != RX_OK)
        {
            goto end;
        }

        /* clang-format off */
        if (strlen("Host") == (key_end - key_begin) 
            && strncasecmp("Host", key_begin, key_end - key_begin) == 0)
        {

            ret = rx_request_process_header_host(&request->host, value_begin,
                                                 value_end - value_begin);

            if (ret != RX_OK)
            {
                goto end;
            }
        }
        else if (strlen("Accept-Encoding") == (key_end - key_begin) 
                 && strncasecmp("Accept-Encoding", 
                                key_begin, 
                                key_end - key_begin) == 0)
        {
            ret = rx_request_process_header_accept_encoding(
                &request->accept_encoding, value_begin,
                value_end - value_begin);

            if (ret != RX_OK)
            {
                goto end;
            }
        }
        else if (strlen("Accept") == (key_end - key_begin)
                 && strncasecmp("Accept", key_begin, key_end - key_begin) == 0)
        {
            ret = rx_request_process_header_accept(
                &request->accept, value_begin, value_end - value_begin);

            if (ret != RX_OK)
            {
                goto end;
            }
        }
        else if (strlen("Content-Length") == (key_end - key_begin)
                 && strncasecmp("Content-Length",
                                key_begin,
                                key_end - key_begin) == 0)
        {
            ret = rx_request_process_header_content_length(
                    &request->content_length, 
                    value_begin, 
                    value_end - value_begin
            );

            if (ret != RX_OK)
            {
                goto end;
            }
        }
        else if (strlen("Content-Type") == (key_end - key_begin)
                 && strncasecmp("Content-Type", 
                                key_begin, 
                                key_end - key_begin) == 0)
        {
            ret = rx_request_process_header_content_type(
                &request->content_type, 
                value_begin,
                value_end - value_begin
            );

            if (ret != RX_OK)
            {
                goto end;
            }
        }

        /* clang-format on */

        key_begin = value_end + 2;
        value_end = strstr(key_begin, "\r\n");
    }

end:
    return ret;
}

int
rx_request_parse_header(
    const char *buffer, size_t len, const char **key, const char **key_end,
    const char **value, const char **value_end
)
{
    int ret;
    const char *colon, *end;

    ret = RX_OK;

    if (buffer == NULL || len == 0)
    {
        ret = RX_ERROR;
        goto end;
    }

    if (key == NULL || key_end == NULL || value == NULL || value_end == NULL)
    {
        ret = RX_ERROR;
        goto end;
    }

    colon = strchr(buffer, ':');
    end   = buffer + len;

    if (colon == NULL || end == NULL)
    {
        ret = RX_ERROR;
        goto end;
    }

    if (colon == buffer || colon == end - 1)
    {
        ret = RX_ERROR;
        goto end;
    }

    if (colon - end >= 2)
    {
        ret = RX_ERROR;
        goto end;
    }

    *key       = buffer;
    *key_end   = colon;
    *value     = colon + 2;
    *value_end = end;

end:
    return ret;
}

int
rx_request_proccess_method(
    rx_request_method_t *method, const char *buffer, size_t len
)
{
    if (buffer == NULL || len == 0)
    {
        *method = RX_REQUEST_METHOD_INVALID;
        return RX_ERROR;
    }

    if (strncmp("GET", buffer, len) == 0)
    {
        *method = RX_REQUEST_METHOD_GET;
    }
    else if (strncmp("POST", buffer, len) == 0)
    {
        *method = RX_REQUEST_METHOD_POST;
    }
    else if (strncmp("PUT", buffer, len) == 0)
    {
        *method = RX_REQUEST_METHOD_PUT;
    }
    else if (strncmp("DELETE", buffer, len) == 0)
    {
        *method = RX_REQUEST_METHOD_DELETE;
    }
    else if (strncmp("HEAD", buffer, len) == 0)
    {
        *method = RX_REQUEST_METHOD_HEAD;
    }
    else
    {
        *method = RX_REQUEST_METHOD_INVALID;
    }
#if defined(RX_DEBUG)
    pthread_t tid = pthread_self();
    char *color   = *method == RX_REQUEST_METHOD_INVALID ? ANSI_COLOR_RED
                                                         : ANSI_COLOR_GREEN;
    char *mark    = *method == RX_REQUEST_METHOD_INVALID ? "x" : "-";

    rx_log(
        LOG_LEVEL_0, LOG_TYPE_DEBUG,
        "[Thread %ld]%4.s%s%3.s%sMethod: %s\n" ANSI_COLOR_RESET, tid, "", mark,
        "", color, rx_request_method_str(*method)
    );
#endif

    return RX_OK;
}

int
rx_request_process_uri(
    struct rx_request_uri *uri, const char *buffer, size_t len
)
{
    int ret;
    char *path_ptr, *query_string_ptr;

    ret      = RX_OK;
    path_ptr = query_string_ptr = NULL;

    if (buffer == NULL)
    {
        uri->result = RX_REQUEST_URI_RESULT_INVALID;
        ret         = RX_ERROR;

        goto end;
    }

    if (len == 0)
    {
        uri->result = RX_REQUEST_URI_RESULT_INVALID;
        ret         = RX_ERROR;

        goto end;
    }

    if (len > RX_MAX_URI_LENGTH)
    {
        uri->result = RX_REQUEST_URI_RESULT_TOO_LONG;
        ret         = RX_ERROR;

        goto end;
    }

    memcpy(uri->raw_uri, buffer, len);
    uri->length       = len;
    uri->raw_uri[len] = '\0';

    path_ptr = strchr(uri->raw_uri, '/');

    if (path_ptr == NULL)
        path_ptr = uri->raw_uri;

    query_string_ptr = strchr(uri->raw_uri, '?');

    if (query_string_ptr != NULL)
    {
        uri->path_end         = query_string_ptr;
        uri->query_string     = query_string_ptr + 1;
        uri->query_string_end = uri->raw_uri + len;
    }
    else
        uri->path_end = uri->raw_uri + len;

    uri->path   = path_ptr;
    uri->result = RX_REQUEST_URI_RESULT_OK;

end:
#if defined(RX_DEBUG)
    pthread_t tid = pthread_self();

    char *color = uri->result != RX_REQUEST_URI_RESULT_OK ? ANSI_COLOR_RED
                                                          : ANSI_COLOR_GREEN;
    char *mark  = uri->result != RX_REQUEST_URI_RESULT_OK ? "x" : "-";

    rx_log(
        LOG_LEVEL_0, LOG_TYPE_DEBUG,
        "[Thread %ld]%4.s%s%3.s%sURI parsing: %s\n"

        ANSI_COLOR_RESET,
        tid, "", mark, "", color,
        uri->result == RX_REQUEST_URI_RESULT_OK ? "OK" : "INVALID"
    );

    rx_log(
        LOG_LEVEL_0, LOG_TYPE_DEBUG, "[Thread %ld]%8.s%sPath: \"%.*s\"\n", tid,
        "", color, (int)(uri->path_end - uri->path), uri->path
    );

    rx_log(
        LOG_LEVEL_0, LOG_TYPE_DEBUG,
        "[Thread %ld]%8.s%sQuery string: \"%.*s\"\n", tid, "", color,
        (int)(uri->query_string_end - uri->query_string), uri->query_string
    );
#endif
    return ret;
}

int
rx_request_process_version(
    struct rx_request_version *version, const char *buffer, size_t len
)
{
    int ret;
    const char *slash, *dot, *end, *major, *minor;

    slash = dot = end = buffer;
    ret               = RX_OK;

    if (buffer == NULL || len == 0)
    {
        version->result = RX_REQUEST_VERSION_RESULT_INVALID;
        ret             = RX_ERROR;
        goto end;
    }

    slash = strchr(buffer, '/');

    if (slash == NULL)
    {
        version->result = RX_REQUEST_VERSION_RESULT_INVALID;
        ret             = RX_ERROR;
        goto end;
    }

    if (strncmp("HTTP", buffer, slash - buffer) != 0)
    {
        version->result = RX_REQUEST_VERSION_RESULT_INVALID;
        ret             = RX_ERROR;

        goto end;
    }

    major = slash + 1;
    dot   = strchr(slash, '.');

    if (dot == NULL)
    {
        version->result = RX_REQUEST_VERSION_RESULT_INVALID;
        ret             = RX_ERROR;

        goto end;
    }

    minor = dot + 1;
    end   = buffer + len;

    if (dot - major != 1 || end - minor != 1)
    {
        version->result = RX_REQUEST_VERSION_RESULT_INVALID;
        ret             = RX_ERROR;

        goto end;
    }

    if (*major != '1' || *minor != '1')
    {
        version->result = RX_REQUEST_VERSION_RESULT_UNSUPPORTED;
        ret             = RX_ERROR;

        goto end;
    }

    version->result = RX_REQUEST_VERSION_RESULT_OK;
    version->major  = atoi(major);
    version->minor  = atoi(minor);

end:
    return ret;
}

int
rx_request_process_header_host(
    struct rx_header_host *host, const char *buffer, size_t len
)
{
    int ret;
    char *colon;

    if (buffer == NULL || len == 0)
    {
        host->result = RX_REQUEST_HEADER_HOST_RESULT_INVALID;
        ret          = RX_ERROR;

        goto end;
    }

    ret = RX_OK;

    memcpy(host->raw_host, buffer, len);
    host->raw_host[len] = '\0';
    host->len           = len;

    colon = strchr(host->raw_host, ':');

    // Colon exists but it is at the first or last position
    if (colon == host->raw_host || colon == host->raw_host + len - 1)

    {
        host->result = RX_REQUEST_HEADER_HOST_RESULT_INVALID;
        ret          = RX_ERROR;

        goto end;
    }

    if (colon == NULL)
    {
        host->raw_host[len]       = ':';
        host->raw_host[len + 1]   = '8';
        host->raw_host[len + 2]   = '0';
        colon                     = host->raw_host + len;
        host->len                 = len + 3;
        host->raw_host[host->len] = '\0';
    }

    host->host     = host->raw_host;
    host->host_end = host->raw_host + (colon - host->raw_host);
    host->port     = colon + 1;
    host->port_end = host->raw_host + host->len;

    if (strncmp("localhost", host->host, host->host_end - host->host) == 0)
    {
        host->result = RX_REQUEST_HEADER_HOST_RESULT_OK;
    }
    else if (strncmp("0.0.0.0", host->host, host->host_end - host->host) == 0)
    {
        host->result = RX_REQUEST_HEADER_HOST_RESULT_OK;
    }
    else if (strncmp("127.0.0.1", host->host, host->host_end - host->host) == 0)
    {
        host->result = RX_REQUEST_HEADER_HOST_RESULT_OK;
    }
    else
    {
        host->result = RX_REQUEST_HEADER_HOST_RESULT_INVALID;
        ret          = RX_ERROR;

        goto end;
    }

    if (host->port != NULL && host->port_end != NULL)
    {
        if (strncmp("8080", host->port, host->port_end - host->port) != 0)
        {
            host->result = RX_REQUEST_HEADER_HOST_RESULT_UNSUPPORTED;
            ret          = RX_ERROR;

            goto end;
        }
        else
        {
            host->result = RX_REQUEST_HEADER_HOST_RESULT_OK;
        }
    }

end:
    return ret;
}

int
rx_request_process_header_accept_encoding(
    struct rx_header_accept_encoding *accept_encoding, const char *buffer,
    size_t len
)
{
    const char *begin, *end, *comma;

    if (buffer == NULL || len == 0)
    {
        accept_encoding->encoding = RX_ENCODING_IDENTITY;
        accept_encoding->qvalue   = 1.0;

        return RX_OK;
    }

    begin = buffer;
    end   = buffer + len;
    comma = strchr(begin, ',');

    while (comma != NULL)
    {
        rx_parse_ae_header(accept_encoding, begin, comma - begin);

        for (comma = comma + 1; *comma == ' '; ++comma)
            ;

        begin = comma;
        comma = strchr(begin, ',');
    }

    if (comma == NULL && begin != end)
    {
        rx_parse_ae_header(accept_encoding, begin, end - begin);
    }

    if (accept_encoding->encoding == RX_ENCODING_UNSET)
    {
        accept_encoding->encoding = RX_ENCODING_IDENTITY;
        accept_encoding->qvalue   = 1.0;
    }

    return RX_OK;
}

int
rx_request_process_header_accept(
    struct rx_qlist *accept, const char *buffer, size_t len
)
{
#if defined(RX_DEBUG)
    pthread_t tid = pthread_self();

    rx_log(
        LOG_LEVEL_0, LOG_TYPE_DEBUG, "[Thread %ld]%4.sAccept header: %.*s\n",
        tid, "", (int)len, buffer
    );
#endif

    const char *begin, *end, *comma;
    size_t offset;

    // Accept header is present but no value is given
    if (buffer == NULL || len == 0)
    {
        rx_qlist_add(accept, "*/*", 3, 1.0);

        return RX_OK;
    }

    begin  = buffer;
    offset = 0;
    end    = buffer + len;
    comma  = rx_strnchr(begin, len, ',');

    while (comma != NULL)
    {
        rx_parse_accept_header(accept, begin, comma - begin);

        for (comma = comma + 1; *comma == ' '; ++comma)
            ;

        offset = comma - buffer - 1;
        begin  = comma;
        comma  = rx_strnchr(buffer + offset + 1, len - offset, ',');
    }

    if (comma == NULL && begin != end)
        rx_parse_accept_header(accept, begin, end - begin);

    return RX_OK;
}

int
rx_request_process_header_content_length(
    size_t *content_length, const char *buffer, size_t len
)
{
#if defined(RX_DEBUG)
    pthread_t tid = pthread_self();

    rx_log(
        LOG_LEVEL_0, LOG_TYPE_DEBUG,
        "[Thread %ld]%4.sContent-Length header: %.*s\n", tid, "", (int)len,
        buffer
    );
#endif

    if (content_length == NULL)
    {
        return RX_ERROR;
    }

    if (buffer == NULL || len == 0)
    {
        *content_length = 0;
        return RX_OK;
    }

    *content_length = atoi(buffer);

    return RX_OK;
}

int
rx_request_process_header_content_type(
    rx_http_mime_t *content_type, const char *buffer, size_t len
)
{
#if defined(RX_DEBUG)
    pthread_t tid = pthread_self();

    rx_log(
        LOG_LEVEL_0, LOG_TYPE_DEBUG,
        "[Thread %ld]%4.sContent-Type header: %.*s ", tid, "", (int)len, buffer
    );
#endif

    if (content_type == NULL)
    {
        return RX_ERROR;
    }

    if (buffer == NULL || len == 0)
    {
        *content_type = RX_HTTP_MIME_TEXT_ALL;
        return RX_OK;
    }

    *content_type = rx_request_mime(buffer, len);

#if defined(RX_DEBUG)
    printf("(Mask = %d)\n", *content_type);
#endif

    return RX_OK;
}

int
rx_request_process_content(
    char *content, size_t content_length, const char *buffer, size_t len
)
{
    if (content == NULL || content_length == 0 || buffer == NULL || len == 0)
    {
        return RX_ERROR;
    }

    memcpy(content, buffer, len);
    content[len] = '\0';

    return RX_OK;
}

const char *
rx_request_method_str(rx_request_method_t method)
{
    switch (method)
    {
    case RX_REQUEST_METHOD_GET:
        return "GET";
    case RX_REQUEST_METHOD_POST:
        return "POST";
    case RX_REQUEST_METHOD_PUT:
        return "PUT";
    case RX_REQUEST_METHOD_DELETE:
        return "DELETE";
    case RX_REQUEST_METHOD_HEAD:
        return "HEAD";
    default:
        return "INVALID";
    }
}

const char *
rx_request_mime_str(rx_http_mime_t mime)
{
    switch (mime)
    {
    case RX_HTTP_MIME_APPLICATION_XFORM:
        return "application/x-www-form-urlencoded";
    case RX_HTTP_MIME_APPLICATION_JSON:
        return "application/json";
    case RX_HTTP_MIME_TEXT_ALL:
        return "text/*";
    case RX_HTTP_MIME_TEXT_HTML:
        return "text/html";
    case RX_HTTP_MIME_TEXT_PLAIN:
        return "text/plain";
    case RX_HTTP_MIME_TEXT_CSS:
        return "text/css";
    case RX_HTTP_MIME_TEXT_JS:
        return "text/javascript";
    case RX_HTTP_MIME_IMAGE_ALL:
        return "image/*";
    case RX_HTTP_MIME_IMAGE_JPEG:
        return "image/jpeg";
    case RX_HTTP_MIME_IMAGE_PNG:
        return "image/png";
    case RX_HTTP_MIME_IMAGE_GIF:
        return "image/gif";
    case RX_HTTP_MIME_ALL:
    default:
        return "*/*";
    }
}

rx_http_mime_t
rx_request_mime(const char *mime_str, size_t len)
{
    if (mime_str == NULL || len == 0 || strncasecmp("*/*", mime_str, 3) == 0)
    {
        return RX_HTTP_MIME_ALL;
    }

    if (strncasecmp("application/x-www-form-urlencoded", mime_str, 33) == 0)
    {
        return RX_HTTP_MIME_APPLICATION_XFORM;
    }

    if (strncasecmp("application/json", mime_str, 16) == 0)
    {
        return RX_HTTP_MIME_APPLICATION_JSON;
    }

    if (strncasecmp("text/*", mime_str, 6) == 0)
    {
        return RX_HTTP_MIME_TEXT_ALL;
    }

    if (strncasecmp("text/html", mime_str, 9) == 0)
    {
        return RX_HTTP_MIME_TEXT_HTML;
    }

    if (strncasecmp("text/plain", mime_str, 10) == 0)
    {
        return RX_HTTP_MIME_TEXT_PLAIN;
    }

    if (strncasecmp("text/css", mime_str, 8) == 0)
    {
        return RX_HTTP_MIME_TEXT_CSS;
    }

    if (strncasecmp("text/javascript", mime_str, 15) == 0)
    {
        return RX_HTTP_MIME_TEXT_JS;
    }

    if (strncasecmp("image/*", mime_str, 7) == 0)
    {
        return RX_HTTP_MIME_IMAGE_ALL;
    }

    if (strncasecmp("image/jpeg", mime_str, 10) == 0)
    {
        return RX_HTTP_MIME_IMAGE_JPEG;
    }

    if (strncasecmp("image/png", mime_str, 9) == 0)
    {
        return RX_HTTP_MIME_IMAGE_PNG;
    }

    if (strncasecmp("image/gif", mime_str, 9) == 0)
    {
        return RX_HTTP_MIME_IMAGE_GIF;
    }

    return RX_HTTP_MIME_ALL;
}

static void
rx_memset_uri(struct rx_request_uri *uri)
{
    memset(uri->raw_uri, 0, sizeof(uri->raw_uri));

    uri->path             = 0;
    uri->path             = uri->raw_uri;
    uri->path_end         = uri->raw_uri;
    uri->query_string     = uri->raw_uri;
    uri->query_string_end = uri->raw_uri;
}

static void
rx_memset_version(struct rx_request_version *version)
{
    version->result = RX_REQUEST_VERSION_RESULT_NONE;
    version->major  = 0;
    version->minor  = 0;
}

static void
rx_memset_header_host(struct rx_header_host *host)
{
    memset(host->raw_host, 0, sizeof(host->raw_host));

    host->result   = RX_REQUEST_HEADER_HOST_RESULT_NONE;
    host->host     = host->raw_host;
    host->host_end = host->raw_host;
    host->port     = NULL;
    host->port_end = NULL;
}

static void
rx_memset_header_accept_encoding(
    struct rx_header_accept_encoding *accept_encoding
)
{
    accept_encoding->encoding = RX_ENCODING_UNSET;
    accept_encoding->qvalue   = 0.0;
}

static void
rx_memset_header_accept(struct rx_qlist *accept)
{
    rx_qlist_create(accept);
}

static void
rx_parse_ae_header(
    struct rx_header_accept_encoding *ae, const char *buffer, size_t len
)
{
    const char *begin, *semi, *end;
    double qvalue;

    begin = buffer;
    end   = buffer + len;
    semi  = strchr(begin, ';');

    if (semi == NULL || semi == end)
    {
        qvalue = 1.0;
        semi   = end;
    }
    else
    {
        qvalue = rx_parse_q_value(semi + 1, end - semi - 1);
    }

    if (qvalue > ae->qvalue)
    {
        if (strncmp("gzip", begin, semi - begin) == 0)
        {
            ae->encoding = RX_ENCODING_GZIP;
            ae->qvalue   = qvalue;
        }
        else if (strncmp("deflate", begin, semi - begin) == 0)
        {
            ae->encoding = RX_ENCODING_DEFLATE;
            ae->qvalue   = qvalue;
        }
        else if (strncmp("br", begin, semi - begin) == 0)
        {
            ae->encoding = RX_ENCODING_BROTLI;
            ae->qvalue   = qvalue;
        }
        else if (strncmp("identity", begin, semi - begin) == 0)
        {
            ae->encoding = RX_ENCODING_IDENTITY;
            ae->qvalue   = qvalue;
        }
        else if (strncmp("*", begin, semi - begin) == 0)
        {
            ae->encoding = RX_ENCODING_ANY;
            ae->qvalue   = qvalue;
        }
        else if (strncmp("compress", begin, semi - begin) == 0)
        {
            ae->encoding = RX_ENCODING_COMPRESS;
            ae->qvalue   = qvalue;
        }
    }

    if (ae->encoding == RX_ENCODING_UNSET)
    {
        ae->encoding = RX_ENCODING_IDENTITY;
        ae->qvalue   = 1.0;
    }
}

static void
rx_parse_accept_header(struct rx_qlist *accept, const char *buf, size_t len)
{
    const char *begin, *semi, *end;
    double qvalue;

    begin = buf;
    end   = buf + len;
    semi  = rx_strnchr(begin, len, ';');

    if (semi == NULL || semi == end)
    {
        qvalue = 1.0;
        semi   = end;
    }
    else
    {
        qvalue = rx_parse_q_value(semi + 1, end - semi - 1);
    }

    rx_qlist_add(accept, begin, semi - begin, qvalue);
}

static double
rx_parse_q_value(const char *buffer, size_t len)
{
    if (buffer == NULL || len == 0)
        return -1;

    const char *begin, *end, *equal;
    char buf[6];
    size_t bufsize;
    double ans;

    begin   = buffer;
    end     = buffer + len;
    equal   = strchr(begin, '=');
    bufsize = 0;
    ans     = 0.0;

    if (equal == NULL || equal == begin || equal == end)
        return -1;

    if (strncmp("q", begin, equal - begin) != 0)
        return -1;

    bufsize = end - equal - 1 > 5 ? 5 : end - equal - 1;
    memcpy(buf, equal + 1, bufsize);
    buf[bufsize] = '\0';

    ans = atof(buf);

    if (ans < 0.0)
        ans = 0.0;
    else if (ans > 1.0)
        ans = 1.0;

    return ans;
}

static int
rx_request_hdrncmp(const char *str, size_t len, const char *key)
{
    return (strlen(key) == len && strncasecmp(key, str, len) == 0);
}
