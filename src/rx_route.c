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
rx_route_get(const char *endpoint, struct rx_route *storage)
{
    int i, ret;

    if (endpoint == NULL || storage == NULL)
        return RX_ERROR;

    for (i = 0; router_table[i].endpoint != NULL; i++)
    {
        if (strcmp(router_table[i].endpoint, endpoint) == 0)
        {
            storage->endpoint = router_table[i].endpoint;
            storage->resource = router_table[i].resource;
            storage->handler  = router_table[i].handler;

            return RX_OK;
        }
    }

    if (strncasecmp(endpoint, "/public/", 8) == 0)
    {
        storage->endpoint = "/public/";
        storage->resource = endpoint;

        memset(&storage->handler, 0, sizeof(struct rx_router_handler));

        storage->handler.get    = rx_route_static;
        storage->handler.post   = NULL;
        storage->handler.put    = NULL;
        storage->handler.patch  = NULL;
        storage->handler.delete = NULL;
        storage->handler.head   = NULL;

        return RX_OK;
    }

    return RX_ERROR;
}
