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
    res->status_code    = RX_HTTP_STATUS_CODE_UNSET;
    res->status_message = NULL;

    res->content        = NULL;
    res->content_length = 0;
    res->content_type   = 0;

    return RX_OK;
}

void
rx_response_destroy(struct rx_response *res)
{
    if (res->status_message != NULL)
        free(res->status_message);

    if (res->content != NULL)
    {
        free(res->content);
        res->content        = NULL;
        res->content_length = 0;
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

const char *
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
