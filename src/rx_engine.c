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
rx_engine_init(struct rx_engine *engine, int argc, const char **argv)
{
    NOOP(argc);
    NOOP(argv);

    engine->server_fd       = -1;
    engine->epoll_fd        = -1;
    engine->server_addr_len = sizeof(engine->server_addr);

    memset(engine->host, 0, sizeof(engine->host));
    memset(engine->service, 0, sizeof(engine->service));
    memset(&engine->server_addr, 0, sizeof(engine->server_addr));
    memset(&engine->events, 0, sizeof(engine->events));

    rx_log(LOG_LEVEL_0, LOG_TYPE_INFO, "Initialize engine... OK\n");

    return RX_OK;
}
