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

int
rx_request_init(struct rx_request *request)
{
    request->method = RX_REQUEST_METHOD_INVALID;

    memset(&request->uri, 0, sizeof(request->uri));
    memset(&request->version, 0, sizeof(request->version));
    memset(&request->host, 0, sizeof(request->host));

    rx_memset_uri(&request->uri);
    rx_memset_version(&request->version);
    rx_memset_header_host(&request->host);

    request->state = RX_REQUEST_STATE_READY;

    return RX_OK;
}

int
rx_request_process_start_line(struct rx_request *request, const char *buffer,
                              size_t len)
{
#if RX_DEBUG
    pthread_t tid = pthread_self();

    rx_log(LOG_LEVEL_0, LOG_TYPE_DEBUG,
           "[Thread %ld]%4.s. Processing starting line: %.*s\n", tid, "", len,
           buffer);
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
rx_request_process_headers(struct rx_request *request, const char *buffer,
                           size_t len)
{
#if defined(RX_DEBUG)
    const char *last;
    size_t cnt    = 0;
    pthread_t tid = pthread_self();
    rx_log(LOG_LEVEL_0, LOG_TYPE_DEBUG,
           "[Thread %ld]%4.s. Processing headers\n", tid, "");
#endif
    NOOP(request);

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
#if defined(RX_DEBUG)
        last = value_end + 2;
        last = (*last == '\r' && *(last + 1) == '\n') ? NULL : last;

        ret = rx_request_parse_header(key_begin, value_end - key_begin, last,
                                      &key_begin, &key_end, &value_begin,
                                      &value_end);
#else
        ret = rx_request_parse_header(key_begin, value_end - key_begin,
                                      &key_begin, &key_end, &value_begin,
                                      &value_end);
#endif

        if (ret != RX_OK)
        {
            goto end;
        }

#if defined(RX_DEBUG)
        cnt++;
#endif
        key_begin = value_end + 2;
        value_end = strstr(key_begin, "\r\n");
    }

end:
#if defined(RX_DEBUG)

    rx_log(LOG_LEVEL_0, LOG_TYPE_DEBUG,
           "[Thread %ld]%4.sProcessed %u headers successfully\n", tid, "", cnt);
#endif

    return ret;
}

#if defined(RX_DEBUG)
int
rx_request_parse_header(const char *buffer, size_t len, const char *last,
                        const char **key, const char **key_end,
                        const char **value, const char **value_end)
#else // !defined(RX_DEBUG)
int
rx_request_parse_header(const char *buffer, size_t len, const char **key,
                        const char **key_end, const char **value,
                        const char **value_end)
#endif
{
#if defined(RX_DEBUG)
    pthread_t tid = pthread_self();

    if (last == NULL)
    {
        rx_log(LOG_LEVEL_0, LOG_TYPE_DEBUG,
               "[Thread %ld]%4.s└── Parsing header buffer: \"%.*s%s\"", tid, "",
               len > 80 ? 80 : len, buffer, len > 80 ? "..." : "");
    }
    else
    {
        rx_log(LOG_LEVEL_0, LOG_TYPE_DEBUG,
               "[Thread %ld]%4.s├── Parsing header buffer: \"%.*s%s\"", tid, "",
               len > 80 ? 80 : len, buffer, len > 80 ? "..." : "");
    }

#endif

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
#if defined(RX_DEBUG)
    if (ret == RX_OK)
    {
        int size = *value_end - *value;
        printf(" -> OK ✅\n");

        if (last == NULL)
        {
            rx_log(LOG_LEVEL_0, LOG_TYPE_DEBUG,
                   "[Thread %ld]%8.s├── Key: \"%.*s\"\n", tid, "",
                   (int)(*key_end - *key), *key);
            rx_log(LOG_LEVEL_0, LOG_TYPE_DEBUG,
                   "[Thread %ld]%8.s└── Value: \"%.*s%s\"\n", tid, "",
                   size > 80 ? 80 : size, *value, size > 80 ? "..." : "");
        }
        else
        {
            rx_log(LOG_LEVEL_0, LOG_TYPE_DEBUG,
                   "[Thread %ld]%4.s│%3.s├── Key: \"%.*s\"\n", tid, "", "",
                   (int)(*key_end - *key), *key);
            rx_log(LOG_LEVEL_0, LOG_TYPE_DEBUG,
                   "[Thread %ld]%4.s│%3.s└── Value: \"%.*s%s\"\n", tid, "", "",
                   size > 80 ? 80 : size, *value, size > 80 ? "..." : "");
        }
    }

    else
        printf(" -> ERROR ❌ (Result: %s)\n",
               ret == RX_ERROR ? "Invalid" : "Unknown");
#endif

    return ret;
}

int
rx_request_proccess_method(rx_request_method_t *method, const char *buffer,
                           size_t len)
{
#if RX_DEBUG
    pthread_t tid = pthread_self();
    rx_log(LOG_LEVEL_0, LOG_TYPE_DEBUG,
           "[Thread %ld]%4.s├── Processing method buffer: \"%.*s\"", tid, "",
           len, buffer);
#endif

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
    if (*method == RX_REQUEST_METHOD_INVALID)
        printf(" -> ERROR ❌ (Result: Invalid method)\n");
    else
        printf(" -> OK ✅\n");

#endif

    return RX_OK;
}

int
rx_request_process_uri(struct rx_request_uri *uri, const char *buffer,
                       size_t len)
{
#if RX_DEBUG
    pthread_t tid = pthread_self();
    rx_log(LOG_LEVEL_0, LOG_TYPE_DEBUG,
           "[Thread %ld]%4.s├── Processing URI buffer: \"%.*s\"", tid, "", len,
           buffer);
#endif

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
    if (ret == RX_OK)
    {
        printf(" -> OK ✅\n");
        rx_log(LOG_LEVEL_0, LOG_TYPE_DEBUG,
               "[Thread %ld]%4.s│%3.s├── Path: %.*s\n", tid, "", "",
               uri->path_end - uri->path, uri->path);
        rx_log(LOG_LEVEL_0, LOG_TYPE_DEBUG,
               "[Thread %ld]%4.s│%3.s└── Query String: %.*s\n", tid, "", "",
               uri->query_string_end - uri->query_string, uri->query_string);
    }
    else
    {
        printf(" -> ERROR ❌ (Result: %s)\n",
               uri->result == RX_REQUEST_URI_RESULT_INVALID    ? "Invalid"
               : uri->result == RX_REQUEST_URI_RESULT_TOO_LONG ? "Too Long"
                                                               : "Unknown");
    }

#endif

    return ret;
}

int
rx_request_process_version(struct rx_request_version *version,
                           const char *buffer, size_t len)
{
#if defined(RX_DEBUG)
    pthread_t tid = pthread_self();
    rx_log(LOG_LEVEL_0, LOG_TYPE_DEBUG,
           "[Thread %ld]%4.s└── Processing version buffer: \"%.*s\"", tid, "",
           len, buffer);
#endif

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
#if defined(RX_DEBUG)
    if (ret == RX_OK)
    {
        printf(" -> OK ✅\n");
        rx_log(LOG_LEVEL_0, LOG_TYPE_DEBUG,
               "[Thread %ld]%8.s├── Protocol: HTTP\n", tid, "", "");
        rx_log(LOG_LEVEL_0, LOG_TYPE_DEBUG, "[Thread %ld]%8.s├── Major: %u\n",
               tid, "", version->major);
        rx_log(LOG_LEVEL_0, LOG_TYPE_DEBUG, "[Thread %ld]%8.s└── Minor: %u\n",
               tid, "", version->minor);
    }
    else
    {
        printf(" -> ERROR ❌ (Result: %s)\n",
               version->result == RX_REQUEST_VERSION_RESULT_INVALID ? "Invalid"
               : version->result == RX_REQUEST_VERSION_RESULT_UNSUPPORTED
                   ? "Unsupported"
                   : "Unknown");
    }

#endif

    return ret;
}

int
rx_request_process_header_host(struct rx_header_host *host, char *buffer)
{
    NOOP(host);
    NOOP(buffer);

    return 0;
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

    host->host = host->raw_host;
    host->port = 0;
}
