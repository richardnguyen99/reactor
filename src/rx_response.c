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
}

int
rx_response_check_mime(struct rx_qlist *accept, const char *mime)
{
    if (accept == NULL || accept->size == 0 || mime == NULL)
        return RX_OK;

    struct rx_qlist_node *node;
    int accept_bits;

    node        = accept->head->next;
    accept_bits = 0;

    while (node != NULL)
    {
        if (strcmp(node->value, "*/*") == 0)
            return RX_OK;

        if (strcmp(node->value, "text/*") == 0)
            accept_bits |= RX_RESPONSE_TEXT_ALL;

        if (strcmp(node->value, "image/*") == 0)
            accept_bits |= RX_RESPONSE_IMAGE_ALL;

        if (strcmp(node->value, mime) == 0)
        {
            if (strcmp(node->value, "text/plain") == 0)
                return RX_OK;
            else if (strcmp(node->value, "text/html") == 0)
                return RX_OK;
            else if (strcmp(node->value, "text/css") == 0)
                return RX_OK;
            else if (strcmp(node->value, "text/javascript") == 0)
                return RX_OK;
        }

        node = node->next;
    }

    if (accept_bits & RX_RESPONSE_TEXT_ALL && strncasecmp("text", mime, 4) == 0)
        return RX_OK;

    if (accept_bits & RX_RESPONSE_IMAGE_ALL &&
        strncasecmp("image", mime, 5) == 0)
        return RX_OK;

    return RX_ERROR;
}

rx_response_mime_t
rx_response_get_content_type(struct rx_qlist *accept, const char *mime)
{
    int accept_bits = 0;
    int type_bits   = 0;
    struct rx_qlist_node *node;

    if (accept == NULL || accept->size == 0)
        return RX_RESPONSE_TEXT_ALL;

    node = accept->head->next;

    if (strncasecmp("text", mime, 4) == 0)
        type_bits |= RX_RESPONSE_TEXT_ALL;
    else if (strncasecmp("image", mime, 5) == 0)
        type_bits |= RX_RESPONSE_IMAGE_ALL;

    while (node != NULL)
    {
        if (strcmp(node->value, "*/*") == 0)
        {
            accept_bits |= RX_RESPONSE_ALL;
        }

        if (strcmp(node->value, "text/*") == 0)
        {
            accept_bits |= RX_RESPONSE_TEXT_ALL;
        }

        if (strcmp(node->value, "image/*") == 0)
        {
            accept_bits |= RX_RESPONSE_IMAGE_ALL;
        }

        if (strcmp(node->value, mime) == 0)
        {
            if (strcmp(node->value, "text/plain") == 0)
                return RX_RESPONSE_TEXT_PLAIN;
            else if (strcmp(node->value, "text/html") == 0)
                return RX_RESPONSE_TEXT_HTML;
            else if (strcmp(node->value, "text/css") == 0)
                return RX_RESPONSE_TEXT_CSS;
            else if (strcmp(node->value, "text/javascript") == 0)
                return RX_RESPONSE_TEXT_JS;
        }

        node = node->next;
    }

    if (accept_bits & RX_RESPONSE_TEXT_ALL)
        return RX_RESPONSE_TEXT_ALL;

    return RX_RESPONSE_NONE;
}

const char *
rx_response_mime_to_string(rx_response_mime_t mime)
{
    switch (mime)
    {
    case RX_RESPONSE_ALL:
        return "*/*";
    case RX_RESPONSE_TEXT_ALL:
        return "text/*";
    case RX_RESPONSE_TEXT_PLAIN:
        return "text/plain";
    case RX_RESPONSE_TEXT_HTML:
        return "text/html";
    case RX_RESPONSE_TEXT_CSS:
        return "text/css";
    case RX_RESPONSE_TEXT_JS:
        return "text/javascript";
    default:
        return NULL;
    }
}

char *
rx_response_status_message(rx_http_status_t status_code)
{
    switch (status_code)
    {
    case RX_HTTP_STATUS_CODE_OK:
        return RX_HTTP_STATUS_MSG_OK;
    case RX_HTTP_STATUS_CODE_BAD_REQUEST:
        return RX_HTTP_STATUS_MSG_BAD_REQUEST;
    case RX_HTTP_STATUS_CODE_NOT_FOUND:
        return RX_HTTP_STATUS_MSG_NOT_FOUND;
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
        rx_log(LOG_LEVEL_0, LOG_TYPE_ERROR, "mmap (at %s:%d): %s\n", __FILE__,
               __LINE__ - 4, strerror(errno));

        goto end;
    }

    buflen = asprintf(&res->content, rx_view_engine.base_template.data, content,
                      file.size);

    if (buflen == -1)
    {
        rx_log(LOG_LEVEL_0, LOG_TYPE_ERROR, "asprintf: %s\n", strerror(errno));

        res->content = NULL;

        goto end;
    }

    res->content_length = buflen;
    res->content_type   = "text/html;charset=utf-8";

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
                              "\r\n";

    pthread_t tid;
    time_t now;
    struct tm *tm;
    char date_buf[128], *buf, *full_buf;
    ssize_t buf_len;

    tid = pthread_self();
    now = time(NULL);
    tm  = gmtime(&now);
    strftime(date_buf, sizeof(date_buf), date_format, tm);
    buf = NULL;

    memset(date_buf, '\0', sizeof(date_buf));

    // Build response headers
    buf_len = asprintf(&buf, headers, res->status_code, res->status_message,
                       res->content_type, res->content_length, date_buf);

    if (buf_len == -1)
    {
        rx_log(LOG_LEVEL_0, LOG_TYPE_ERROR,
               "[Thread %ld]%4.sasprintf() failed to allocate "
               "memory for response buffer",
               tid, "");

        return RX_ERROR;
    }

    // Build full response buffer
    full_buf = malloc((size_t)buf_len + res->content_length + 1);

    if (full_buf == NULL)
    {
        rx_log(LOG_LEVEL_0, LOG_TYPE_ERROR,
               "[Thread %ld]%4.sfailed to allocate memory for "
               "response buffer",
               tid, "");

        return RX_ERROR;
    }

    // Copy headers and content into full buffer
    memcpy(full_buf, buf, (size_t)buf_len);

    // Copy content into full buffer. The content should be available from
    // router at this point.
    memcpy(full_buf + buf_len, res->content, res->content_length);
    full_buf[buf_len + res->content_length] = '\0';
    buf_len += res->content_length;

    free(buf);

    res->is_resp_alloc   = 1;
    res->resp_buf        = full_buf;
    res->resp_buf_offset = 0;
    res->resp_buf_size   = (size_t)buf_len;

    return RX_OK;
}
