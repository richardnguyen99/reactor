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

#ifndef __RX_VIEW_H__
#define __RX_VIEW_H__ 1

#include <rx_config.h>
#include <rx_core.h>

struct rx_map_file
{
    struct rx_file file;
    char *data;
};

struct rx_view
{
    struct rx_map_file base_template;
    struct rx_map_file client_error_template;
    struct rx_map_file server_error_template;
};

extern struct rx_view rx_view_engine;
extern const char *const rx_view_base_client_error;
extern const char *const rx_view_base_server_error;
extern const char *const rx_view_header_template;

int
rx_view_init();

int
rx_view_load_template(const char *path);

int
rx_view_load_4xx(const char *path);

int
rx_view_load_5xx(const char *path);

void
rx_view_destroy();

#endif /* __RX_VIEW_H__ */
