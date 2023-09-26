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
rx_request_proccess_method(rx_request_method_t *method, const char *buffer)
{
    size_t len = strlen(buffer);

    if (buffer == NULL || len == 0)
    {
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
rx_request_process_uri(struct rx_request_uri *uri, const char *buffer)
{
    if (buffer == NULL)
    {
        return RX_ERROR;
    }

    char *query_string_ptr;
    size_t len;

    len              = strlen(buffer);
    query_string_ptr = NULL;

    if (len > RX_MAX_URI_LENGTH)
    {
        return RX_ERROR;
    }

    memcpy(uri->raw_uri, buffer, len);

    uri->path        = uri->raw_uri;
    query_string_ptr = strchr(uri->raw_uri, '?');

    if (query_string_ptr != NULL)
    {
        uri->path_end     = query_string_ptr;
        uri->query_string = query_string_ptr + 1;
    }

    return RX_OK;
}

int
rx_request_process_version(struct rx_request_version *version, char *buffer)
{
    NOOP(version);
    NOOP(buffer);

    return 0;
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
    version->major = 1;
    version->minor = 1;
}

static void
rx_memset_header_host(struct rx_header_host *host)
{
    memset(host->raw_host, 0, sizeof(host->raw_host));

    host->host = host->raw_host;
    host->port = 0;
}
