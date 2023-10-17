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

#ifndef __RX_ROUTE_H__
#define __RX_ROUTE_H__ 1

#include <rx_config.h>
#include <rx_core.h>

/* ### Handlers for each HTTP endpoint
 *
 * Every endpoint will support these following HTTP methods:
 *
 * - GET
 * - POST
 * - PUT
 * - PATCH
 * - DELETE
 * - HEAD
 *
 * Each handler will be called when the corresponding HTTP method is used. If a
 * request is made with an unsupported HTTP method, the server will respond with
 * a 405 status code (Method Not Allowed).
 */
struct rx_router_handler
{
    void *(*get)(struct rx_request *req, struct rx_response *res);
    void *(*post)(struct rx_request *req, struct rx_response *res);
    void *(*put)(struct rx_request *req, struct rx_response *res);
    void *(*patch)(struct rx_request *req, struct rx_response *res);
    void *(*delete)(struct rx_request *req, struct rx_response *res);
    void *(*head)(struct rx_request *req, struct rx_response *res);
};

struct rx_route
{
    struct rx_router_handler handler;

    const char *endpoint;
    const char *resource;
};

extern const struct rx_route router_table[];

int
rx_route_get(struct rx_route *storage, const char *endpoint, size_t ep_len);

void *
rx_route_static(struct rx_request *req, struct rx_response *res);

void *
rx_route_4xx(struct rx_request *req, struct rx_response *res, int code);

void *
rx_route_index_get(struct rx_request *req, struct rx_response *res);

void *
rx_route_about_get(struct rx_request *req, struct rx_response *res);

void *
rx_route_static(struct rx_request *req, struct rx_response *res);

#endif /* __RX_ROUTE_H__ */
