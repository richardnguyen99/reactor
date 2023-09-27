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
           "[Thread %ld]%4.sProcessing starting line: %.*s\n", tid, "", len,
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
rx_request_proccess_method(rx_request_method_t *method, const char *buffer,
                           size_t len)
{
#if RX_DEBUG
    pthread_t tid = pthread_self();
    rx_log(LOG_LEVEL_0, LOG_TYPE_DEBUG,
           "[Thread %ld]%8.sProcessing method: %.*s\n", tid, "", len, buffer);
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

    return RX_OK;
}

int
rx_request_process_uri(struct rx_request_uri *uri, const char *buffer,
                       size_t len)
{
#if RX_DEBUG
    pthread_t tid = pthread_self();
    rx_log(LOG_LEVEL_0, LOG_TYPE_DEBUG,
           "[Thread %ld]%8.sProcessing URI: %.*s\n", tid, "", len, buffer);
#endif

    if (buffer == NULL)
    {
        return RX_ERROR;
    }

    char *path_ptr;
    char *query_string_ptr;

    path_ptr         = NULL;
    query_string_ptr = NULL;

    if (len == 0)
    {
        uri->result = RX_REQUEST_URI_RESULT_INVALID;
        return RX_ERROR;
    }

    if (len > RX_MAX_URI_LENGTH)
    {
        uri->result = RX_REQUEST_URI_RESULT_TOO_LONG;
        return RX_ERROR;
    }

    memcpy(uri->raw_uri, buffer, len);
    uri->length       = len;
    uri->raw_uri[len] = '\0';

    path_ptr = strchr(uri->raw_uri, '/');

    if (path_ptr == NULL)
    {
        path_ptr = uri->raw_uri;
    }

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

    return RX_OK;
}

int
rx_request_process_version(struct rx_request_version *version,
                           const char *buffer, size_t len)
{
#if defined(RX_DEBUG)
    pthread_t tid = pthread_self();
    rx_log(LOG_LEVEL_0, LOG_TYPE_DEBUG,
           "[Thread %ld]%8.sProcessing version: %.*s\n", tid, "", len, buffer);
#endif
    if (buffer == NULL || len == 0)
    {
        version->result = RX_REQUEST_VERSION_RESULT_INVALID;
        return RX_ERROR;
    }

    const char *slash, *dot, *end, *major, *minor;

    slash = dot = end = buffer;

    slash = strchr(buffer, '/');

    if (slash == NULL)
    {
        version->result = RX_REQUEST_VERSION_RESULT_INVALID;
        return RX_ERROR;
    }

    if (strncmp("HTTP", buffer, slash - buffer) != 0)
    {
        version->result = RX_REQUEST_VERSION_RESULT_INVALID;
        return RX_ERROR;
    }

    major = slash + 1;
    dot   = strchr(slash, '.');

    if (dot == NULL)
    {
        version->result = RX_REQUEST_VERSION_RESULT_INVALID;
        return RX_ERROR;
    }

    minor = dot + 1;
    end   = buffer + len;

    if (dot - major != 1 || end - minor != 1)
    {
        version->result = RX_REQUEST_VERSION_RESULT_INVALID;
        return RX_ERROR;
    }

    if (*major != '1' || *minor != '1')
    {
        version->result = RX_REQUEST_VERSION_RESULT_UNSUPPORTED;
        return RX_ERROR;
    }

    version->result = RX_REQUEST_VERSION_RESULT_OK;
    version->major  = *major - '0';
    version->minor  = *minor - '0';

    return RX_OK;
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
