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

#ifndef __RX_RESPONSE_H__
#define __RX_RESPONSE_H__ 1

#include <rx_config.h>
#include <rx_core.h>

enum RX_RESPONSE_SUPPORTED_MIME
{
    RX_RESPONSE_NONE       = 0x000000000,
    RX_RESPONSE_ALL        = 0x000000010,
    RX_RESPONSE_TEXT_ALL   = 0x000000008,
    RX_RESPONSE_TEXT_PLAIN = 0x000000009,
    RX_RESPONSE_TEXT_HTML  = 0x00000000A,
    RX_RESPONSE_TEXT_CSS   = 0x00000000B,
    RX_RESPONSE_TEXT_JS    = 0x00000000C,
    RX_RESPONSE_IMAGE_ALL  = 0x000000020,
    RX_RESPONSE_IMAGE_ICO  = 0x000000011,
};

typedef enum RX_RESPONSE_SUPPORTED_MIME rx_response_mime_t;

struct rx_response
{
    // struct rx_http_version version;
    rx_http_status_t status_code;
    char *status_message;

    int is_content_mmapd;
    char *content;
    size_t content_length;
    char *content_type;

    int is_resp_alloc;
    char *resp_buf;
    size_t resp_buf_offset;
    size_t resp_buf_size;
};

int
rx_response_init(struct rx_response *response);

void
rx_response_destroy(struct rx_response *response);

int
rx_response_check_mime(struct rx_qlist *accept, const char *mime);

rx_response_mime_t
rx_response_get_content_type(struct rx_qlist *accept, const char *ext);

const char *
rx_response_mime_to_string(rx_response_mime_t mime);

char *
rx_response_status_message(rx_http_status_t status_code);

void
rx_response_render(struct rx_response *response, const char *path);

int
rx_response_construct(struct rx_response *response);

#endif /* __RX_RESPONSE_H__ */
