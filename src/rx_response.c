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

int
rx_response_init(struct rx_response *res)
{
    if (res == NULL)
    {
        return RX_ERROR;
    }

    res->status_code    = RX_HTTP_STATUS_CODE_UNSET;
    res->status_message = NULL;

    res->location = NULL;

    res->is_content_mmapd = 0;
    res->content          = NULL;
    res->content_length   = 0;
    res->content_type     = 0;

    res->is_resp_alloc   = 0;
    res->resp_buf        = NULL;
    res->resp_buf_offset = 0;
    res->resp_buf_size   = 0;

    return RX_OK;
}

void
rx_response_destroy(struct rx_response *res)
{
    if (res->content != NULL)
    {
        if (res->is_content_mmapd == 1)
            munmap(res->content, res->content_length);
        else
            free(res->content);

        res->content        = NULL;
        res->content_length = 0;
    }

    if (res->resp_buf != NULL && res->is_resp_alloc == 1)
    {
        free(res->resp_buf);
        res->resp_buf        = NULL;
        res->resp_buf_offset = 0;
        res->resp_buf_size   = 0;
    }

    if (res->location != NULL)
    {
        free(res->location);
        res->location = NULL;
    }

    if (res->last_modified != NULL)
    {
        free(res->last_modified);
        res->location = NULL;
    }
}

const char *
rx_response_mime_to_string(rx_http_mime_t mime)
{
    switch (mime)
    {
    case RX_HTTP_MIME_TEXT_PLAIN:
        return RX_HTTP_MIME_TEXT_PLAIN_STR;
    case RX_HTTP_MIME_TEXT_HTML:
        return RX_HTTP_MIME_TEXT_HTML_STR;
    case RX_HTTP_MIME_TEXT_CSS:
        return RX_HTTP_MIME_TEXT_CSS_STR;
    case RX_HTTP_MIME_TEXT_JS:
        return RX_HTTP_MIME_TEXT_JS_STR;
    case RX_HTTP_MIME_APPLICATION_XML:
        return RX_HTTP_MIME_APPLICATION_XML_STR;
    case RX_HTTP_MIME_APPLICATION_JSON:
        return RX_HTTP_MIME_APPLICATION_JSON_STR;
    case RX_HTTP_MIME_APPLICATION_XHTML:
        return RX_HTTP_MIME_APPLICATION_XHTML_STR;
    case RX_HTTP_MIME_APPLICATION_XFORM:
        return RX_HTTP_MIME_APPLICATION_XFORM_STR;
    case RX_HTTP_MIME_IMAGE_ICO:
        return RX_HTTP_MIME_IMAGE_ICO_STR;
    case RX_HTTP_MIME_IMAGE_GIF:
        return RX_HTTP_MIME_IMAGE_GIF_STR;
    case RX_HTTP_MIME_IMAGE_JPEG:
        return RX_HTTP_MIME_IMAGE_JPEG_STR;
    case RX_HTTP_MIME_IMAGE_PNG:
        return RX_HTTP_MIME_IMAGE_PNG_STR;
    case RX_HTTP_MIME_IMAGE_SVG:
        return RX_HTTP_MIME_IMAGE_SVG_STR;
    default:
        return RX_HTTP_MIME_TEXT_OCTET_STREAM_STR;
    }
}

char *
rx_response_status_message(rx_http_status_t status_code)
{
    switch (status_code)
    {
    case RX_HTTP_STATUS_CODE_OK:
        return RX_HTTP_STATUS_MSG_OK;
    case RX_HTTP_STATUS_CODE_NOT_MODIFIED:
        return RX_HTTP_STATUS_MSG_NOT_MODIFIED;
    case RX_HTTP_STATUS_CODE_BAD_REQUEST:
        return RX_HTTP_STATUS_MSG_BAD_REQUEST;
    case RX_HTTP_STATUS_CODE_NOT_FOUND:
        return RX_HTTP_STATUS_MSG_NOT_FOUND;
    case RX_HTTP_STATUS_CODE_FOUND:
        return RX_HTTP_STATUS_MSG_FOUND;
    case RX_HTTP_STATUS_CODE_METHOD_NOT_ALLOWED:
        return RX_HTTP_STATUS_MSG_METHOD_NOT_ALLOWED;
    case RX_HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR:
        return RX_HTTP_STATUS_MSG_INTERNAL_SERVER_ERROR;
    default:
        return RX_HTTP_STATUS_CODE_UNSET;
    }
}

void
rx_response_render(struct rx_response *res, const char *path)
{

    int ret, buflen;
    struct rx_file file;
    char *content;

    memset(&file, 0, sizeof(file));
    ret = rx_file_open(&file, path, O_RDONLY);

    if (ret == RX_ERROR)
    {
        rx_log(LOG_LEVEL_0, LOG_TYPE_ERROR, "Failed to open file\n");

        goto end;
    }

    content = mmap(NULL, file.size, PROT_READ, MAP_PRIVATE, file.fd, 0);

    if (content == MAP_FAILED)
    {
        rx_log(
            LOG_LEVEL_0, LOG_TYPE_ERROR, "mmap (at %s:%d): %s\n", __FILE__,
            __LINE__ - 4, strerror(errno)
        );

        goto end;
    }

    buflen = asprintf(
        &res->content, rx_view_engine.base_template.data, content, file.size
    );

    if (buflen == -1)
    {
        rx_log(LOG_LEVEL_0, LOG_TYPE_ERROR, "asprintf: %s\n", strerror(errno));

        res->content = NULL;

        goto end;
    }

    res->content_length = buflen;
    res->content_type   = RX_HTTP_MIME_TEXT_HTML;

    res->status_code = RX_HTTP_STATUS_CODE_OK;
    res->status_message =
        (char *)rx_response_status_message(RX_HTTP_STATUS_CODE_OK);

    if (munmap(content, file.size) == -1)
    {
        rx_log(LOG_LEVEL_0, LOG_TYPE_ERROR, "munmap: %s\n", strerror(errno));
    }

end:
    rx_file_close(&file);
}

void
rx_response_send(struct rx_response *res, const char *msg, size_t len)
{
    char *buf = malloc(len + 1);

    if (buf == NULL)
    {
        rx_log(
            LOG_LEVEL_0, LOG_TYPE_ERROR, "%s malloc: %s\n", __func__,
            strerror(errno)
        );

        return;
    }

    memcpy(buf, msg, len);
    buf[len] = '\0';

    if (res->content_type == RX_HTTP_MIME_NONE)
        res->content_type = RX_HTTP_MIME_TEXT_PLAIN;

    res->content        = buf;
    res->content_length = len;
    res->status_code    = RX_HTTP_STATUS_CODE_OK;
    res->status_message =
        (char *)rx_response_status_message(RX_HTTP_STATUS_CODE_OK);
}

void
rx_response_redirect(struct rx_response *res, const char *location)
{

    /* Default status code for redirect is 302 (Found), unless set otherwise */

    res->status_code = res->status_code == RX_HTTP_STATUS_CODE_UNSET
                           ? RX_HTTP_STATUS_CODE_FOUND
                           : res->status_code;
    res->status_message =
        (char *)rx_response_status_message(RX_HTTP_STATUS_CODE_FOUND);

    res->location = malloc(strlen(location) + 1);

    if (res->location == NULL)
    {
        rx_log(
            LOG_LEVEL_0, LOG_TYPE_ERROR, "%s malloc: %s\n", __func__,
            strerror(errno)
        );

        return;
    }

    strcpy(res->location, location);

    /* Content, by default, is not set. If the caller wishes to add content to
       a redirect response, it should construct the body itself before calling
       this function.
     */

    res->content_type =
        RX_HTTP_MIME_NONE ? RX_HTTP_MIME_TEXT_HTML : res->content_type;
}

int
rx_response_construct(struct rx_response *res)
{
    // HTTP Date format
    const char *date_format = "%a, %d %b %Y %H:%M:%S %Z";
    const char *headers     = "HTTP/1.1 %d %s\r\n"
                              "Server: Reactor\r\n"
                              "Content-Type: %s\r\n"
                              "Content-Length: %zu\r\n"
                              "Date: %s\r\n"
                              "Connection: close\r\n"
                              "%s" /* Additional headers */
                          "\r\n";

    pthread_t tid;
    time_t now;
    struct tm *tm;
    char date_buf[128], extra_header_buf[2048], *buf, *full_buf;
    ssize_t buf_len, ehb_offset;

    memset(extra_header_buf, '\0', sizeof(extra_header_buf));

    const rx_http_status_t status_code = res->status_code;
    const char *status_message  = rx_response_status_message(status_code);
    const char *content_type    = rx_response_mime_to_string(res->content_type);
    const size_t content_length = res->content_length;

    tid        = pthread_self();
    now        = time(NULL);
    tm         = gmtime(&now);
    ehb_offset = 0;
    buf        = NULL;
    strftime(date_buf, sizeof(date_buf), date_format, tm);

    if (res->location)
    {
        ehb_offset += snprintf(
            extra_header_buf + ehb_offset, 1024 - ehb_offset,
            "Location: %s\r\n", res->location
        );
    }

    if (res->last_modified)
    {
        char last_modified[128];
        time_t secs = res->last_modified->tv_sec;
        struct tm lmtm;

        gmtime_r(&secs, &lmtm);
        strftime(last_modified, sizeof(last_modified), date_format, &lmtm);

        ehb_offset += snprintf(
            extra_header_buf + ehb_offset, 1024 - ehb_offset,
            "Last-Modified: %s\r\n", last_modified
        );
    }

    extra_header_buf[ehb_offset] = '\0';

    // Build response headers
    buf_len = asprintf(
        &buf, headers, status_code, status_message, content_type,
        content_length, date_buf, extra_header_buf
    );

    if (buf_len == -1)
    {
        rx_log(
            LOG_LEVEL_0, LOG_TYPE_ERROR,
            "[Thread %ld]%4.sasprintf() failed to allocate "
            "memory for response buffer",
            tid, ""
        );

        return RX_ERROR;
    }

    // Build full response buffer
    full_buf = malloc((size_t)buf_len + res->content_length + 1);

    if (full_buf == NULL)
    {
        rx_log(
            LOG_LEVEL_0, LOG_TYPE_ERROR,
            "[Thread %ld]%4.sfailed to allocate memory for "
            "response buffer",
            tid, ""
        );

        return RX_ERROR;
    }

    // Copy headers and content into full buffer
    memcpy(full_buf, buf, (size_t)buf_len);

    // Copy content into full buffer. The content should be available from
    // router at this point.

    if (res->content != NULL)
    {
        memcpy(full_buf + buf_len, res->content, res->content_length);
        full_buf[buf_len + res->content_length] = '\0';
        buf_len += res->content_length;
    }

    free(buf);

    res->is_resp_alloc   = 1;
    res->resp_buf        = full_buf;
    res->resp_buf_offset = 0;
    res->resp_buf_size   = (size_t)buf_len;

    return RX_OK;
}
