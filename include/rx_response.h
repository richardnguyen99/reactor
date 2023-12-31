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

struct rx_response
{
    rx_http_status_t status_code;
    char *status_message;

    char *location;
    struct timespec *last_modified;

    int is_content_mmapd;
    char *content;
    size_t content_length;
    rx_http_mime_t content_type;

    int is_resp_alloc;
    char *resp_buf;
    size_t resp_buf_offset;
    size_t resp_buf_size;
};

int
rx_response_init(struct rx_response *response);

void
rx_response_destroy(struct rx_response *response);

char *
rx_response_status_message(rx_http_status_t status_code);

void
rx_response_send(struct rx_response *response, const char *msg, size_t len);

void
rx_response_render(struct rx_response *response, const char *path);

void
rx_response_redirect(struct rx_response *response, const char *location);

int
rx_response_construct(struct rx_response *response);

#endif /* __RX_RESPONSE_H__ */
