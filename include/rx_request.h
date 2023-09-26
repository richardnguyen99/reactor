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

#ifndef __RX_REQUEST_H__
#define __RX_REQUEST_H__ 1

#include <rx_config.h>
#include <rx_core.h>

#define RX_MAX_HEADER_LENGTH 1024

enum rx_request_uri_result
{
    RX_REQUEST_URI_RESULT_OK,
    RX_REQUEST_URI_RESULT_INVALID,
    RX_REQUEST_URI_RESULT_TOO_LONG,
};

enum rx_request_state
{
    RX_REQUEST_STATE_READY,
    RX_REQUEST_STATE_METHOD,
    RX_REQUEST_STATE_URI,
    RX_REQUEST_STATE_VERSION,
    RX_REQUEST_STATE_HEADER,
    RX_REQUEST_STATE_BODY,
    RX_REQUEST_STATE_DONE,
};

enum rx_request_method
{
    RX_REQUEST_METHOD_INVALID,
    RX_REQUEST_METHOD_GET,
    RX_REQUEST_METHOD_POST,
    RX_REQUEST_METHOD_PUT,
    RX_REQUEST_METHOD_DELETE,
    RX_REQUEST_METHOD_HEAD,
};

typedef enum rx_request_state rx_request_state_t;
typedef enum rx_request_method rx_request_method_t;
typedef enum rx_request_uri_result rx_request_uri_result_t;

/* Structure to store the URI of an HTTP request

    # Example

    If a request is made with the URI:

    ```http
    GET /index.html?foo=bar&baz=waldo HTTP/1.1
    ...
    ```

    Then the `struct rx_request_uri` will be populated as follows:

    ```c
    struct rx_request_uri uri = {
        .raw_uri = "/index.html?foo=bar&baz=waldo",
        .length = 29,
        .path = .raw_uri,
        .path_end = .path + 11,
        .query_string = .raw_uri + 12 ,
        .query_string_end = .query_string + 17,
    };
    ```

    # Usage

    ```c
    struct rx_request_uri uri_struct;
    const char *uri = "/index.html?foo=bar&baz=waldo";
    int ret = rx_request_process_uri(&uri_struct, uri);
    ```
*/
struct rx_request_uri
{
    /* Result of the URI processing

        Valid results:
            - `RX_REQUEST_URI_RESULT_OK`: The URI is valid
            - `RX_REQUEST_URI_RESULT_INVALID`: The URI is invalid
            - `RX_REQUEST_URI_RESULT_TOO_LONG`: The URI is too long
    */
    rx_request_uri_result_t result;

    /* Main buffeer to store the raw URI in an HTTP request

        If a request is made with the URI:

        ```http
        GET /index.html?foo=bar HTTP/1.1
        ```

        Then the raw URI will be: `"/index.html?foo=bar"`*/
    char raw_uri[RX_MAX_URI_LENGTH];

    /* Length of the main URI buffer */
    size_t length;

    /* The path of the URI

        If a request is made with the URI:

        ```http
        GET /index.html?foo=bar HTTP/1.1
        ```

        The path pointer will point to the string `"/index.html"`. However, the
        end of the string is marked by `path_end` pointer*/
    char *path;

    /* Pointer that marks the end of the path string */
    char *path_end;

    /* The query string of the URI

        If a request is made with the URI:

        ```http
        GET /index.html?foo=bar&baz=waldo HTTP/1.1
        ```

        The `query_string` will point to the string `"foo=bar&baz=waldo"`.
        However, the end of the string is marked by `query_string_end` pointer
     */
    char *query_string;

    /* Pointer that marks the end of the query string */
    char *query_string_end;
};

struct rx_request_version
{
    uint8_t major;
    uint8_t minor;
};

struct rx_header_host
{
    char raw_host[RX_MAX_HEADER_LENGTH];
    char *host;
    char *host_end;
    char *port;
    char *port_end;
};

struct rx_header_user_agent
{
    char raw_user_agent[RX_MAX_HEADER_LENGTH];
};

struct rx_header_accept
{
    char raw_accept[RX_MAX_HEADER_LENGTH];
};

struct rx_request
{
    rx_request_state_t state;
    rx_request_method_t method;
    struct rx_request_uri uri;
    struct rx_request_version version;

    struct rx_header_host host;
    struct rx_header_user_agent user_agent;
    struct rx_header_accept accept;
};

int
rx_request_init(struct rx_request *request);

int
rx_request_proccess_method(rx_request_method_t *method, const char *buffer);

int
rx_request_process_uri(struct rx_request_uri *uri, const char *buffer);

int
rx_request_process_version(struct rx_request_version *version, char *buffer);

int
rx_request_process_header_host(struct rx_header_host *host, char *buffer);

#endif /* __RX_REQUEST_H__ */
