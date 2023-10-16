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

#ifndef __RX_ENGINE_H__
#define __RX_ENGINE_H__ 1

#include <rx_config.h>
#include <rx_core.h>

struct rx_engine
{
    int server_fd;
    int epoll_fd;

    struct sockaddr_in server_addr;
    socklen_t server_addr_len;
    char host[NI_MAXHOST];
    char service[NI_MAXSERV];

    struct epoll_event events[RX_MAX_EVENTS];
};

/* Construct the reactor engine.

   The function initializes the reactor engine with the given arguments from the
   command line.
 */
int
rx_engine_init(struct rx_engine *engine, int argc, const char **argv);

/* Prepare the HTTP server.

   The function prepares the HTTP based on TCP connection with a passive
   (listening) socket.
*/
int
rx_engine_prepare_server(struct rx_engine *engine);

/* Prepare the epoll instance for monitoring file descriptors.

   The function initializes a Linux `epoll` instance for monitoring file
   desriptors, both passive (listening) and active (connecting) sockets.
 */
int
rx_engine_prepare_epoll(struct rx_engine *engine);

/* Construct a thread pool for handling HTTP requests.

   The function constructs a pool of threads that will be processing HTTP
   requests and responses.
 */
int
rx_engine_prepare_pool(struct rx_engine *engine);

/* Construct a bounded buffer for storing HTTP requests.

   The function constructs a ring buffer data structure for storing tasks that
   are waiting for a thread to pull from the buffer and process.

   This buffer is accessible by all of the threads. Therefore, it requires a
   synchronization mechanism to ensure that the buffer is thread-safe.
 */
int
rx_engine_prepare_ring_buffer(struct rx_engine *engine);

/* Bind the reactor engine.

   The function binds the reactor engine to the given address and port.
 */
int
rx_engine_bind(struct rx_engine *engine);

/* Destroy the reactor engine.

   Clean up the reactor engine and free all of its resources.
 */
int
rx_engine_destroy(struct rx_engine *engine);

/* Run the reactor engine.

   The function runs the reactor engine as the main loop of the program. It is
   supposed to be called after all of the initialization steps are done, and
   continue to run unless an error occurs or the program is terminated.
 */
int
rx_engine_run(struct rx_engine *engine);

/* Get the error string from the error code.

   The function returns the error string from the given error code.
 */
const char *
rx_engine_strerror(int code);

#endif /* __RX_ENGINE_H__ */
