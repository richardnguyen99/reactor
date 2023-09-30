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

#include <unity/unity.h>
#include <unity/unity_fixture.h>

#include <rx_config.h>
#include <rx_core.h>

struct rx_header_host header;

static void
rx_memset_header_host(struct rx_header_host *host)
{
    memset(host->raw_host, 0, sizeof(host->raw_host));

    host->result   = RX_REQUEST_HEADER_HOST_RESULT_NONE;
    host->host     = NULL;
    host->host_end = NULL;
    host->port     = NULL;
    host->port_end = NULL;
}

TEST_GROUP(RX_REQUEST_HOST_HEADER);

TEST_SETUP(RX_REQUEST_HOST_HEADER)
{
}

TEST_TEAR_DOWN(RX_REQUEST_HOST_HEADER)
{
}

TEST(RX_REQUEST_HOST_HEADER, EmptyBufferHostHeaderTest)
{
    const char *buffer = "";
    int result =
        rx_request_process_header_host(&header, buffer, strlen(buffer));

    TEST_ASSERT_EQUAL_INT(RX_ERROR, result);
    TEST_ASSERT_EQUAL_INT(RX_REQUEST_HEADER_HOST_RESULT_INVALID, header.result);

    TEST_PASS_MESSAGE("Empty buffer host header test passed");
}

TEST(RX_REQUEST_HOST_HEADER, NullBufferHostHeaderTest)
{
    const char *buffer = NULL;
    int result         = rx_request_process_header_host(&header, buffer, 10);

    TEST_ASSERT_EQUAL_INT(RX_ERROR, result);
    TEST_ASSERT_EQUAL_INT(RX_REQUEST_HEADER_HOST_RESULT_INVALID, header.result);

    TEST_PASS_MESSAGE("Null buffer host header test passed");
}

TEST(RX_REQUEST_HOST_HEADER, ColonAtFirstHostTest)
{
    const char *buffer = ":8080";
    int result         = rx_request_process_header_host(&header, buffer, 1);

    TEST_ASSERT_EQUAL_INT(RX_ERROR, result);
    TEST_ASSERT_EQUAL_INT(RX_REQUEST_HEADER_HOST_RESULT_INVALID, header.result);

    TEST_PASS_MESSAGE("Colon at first host test passed");
}

TEST(RX_REQUEST_HOST_HEADER, MissingPortValueHostTest)
{
    const char *buffer = "localhost:";
    int result         = rx_request_process_header_host(&header, buffer, 10);

    TEST_ASSERT_EQUAL_INT(RX_ERROR, result);
    TEST_ASSERT_EQUAL_INT(RX_REQUEST_HEADER_HOST_RESULT_INVALID, header.result);

    TEST_PASS_MESSAGE("Missing port value host test passed");
}

TEST(RX_REQUEST_HOST_HEADER, SimpleValidHostTest)
{
    const char *buffer = "localhost";
    int result         = rx_request_process_header_host(&header, buffer, 9);

    TEST_ASSERT_EQUAL_INT(RX_OK, result);
    TEST_ASSERT_EQUAL_INT(RX_REQUEST_HEADER_HOST_RESULT_OK, header.result);
    TEST_ASSERT_EQUAL_STRING_LEN("localhost", header.host,
                                 header.host_end - header.host);
    TEST_ASSERT_EQUAL_STRING_LEN("80", header.port,
                                 header.port_end - header.port);

    TEST_PASS_MESSAGE("Simple valid host test passed");
}

TEST(RX_REQUEST_HOST_HEADER, LongValidHostTest)
{
    const char *buffer = "localhost:8080";
    int result =
        rx_request_process_header_host(&header, buffer, strlen(buffer));

    TEST_ASSERT_EQUAL_INT(RX_OK, result);
    TEST_ASSERT_EQUAL_INT(RX_REQUEST_HEADER_HOST_RESULT_OK, header.result);

    TEST_ASSERT_EQUAL(strlen("localhost"), header.host_end - header.host);
    TEST_ASSERT_EQUAL_STRING_LEN("localhost", header.host,
                                 header.host_end - header.host);

    TEST_ASSERT_EQUAL(strlen("8080"), header.port_end - header.port);
    TEST_ASSERT_EQUAL_STRING_LEN("8080", header.port,
                                 header.port_end - header.port);

    TEST_PASS_MESSAGE("Long valid host test passed");
}

TEST(RX_REQUEST_HOST_HEADER, LoopbackAddressHostTest)
{
    const char *buffer    = "127.0.0.1:8080";
    const size_t addr_len = strlen("127.0.0.1");
    const size_t port_len = strlen("8080");

    int result =
        rx_request_process_header_host(&header, buffer, strlen(buffer));

    TEST_ASSERT_EQUAL_INT(RX_OK, result);
    TEST_ASSERT_EQUAL_INT(RX_REQUEST_HEADER_HOST_RESULT_OK, header.result);

    TEST_ASSERT_EQUAL(addr_len, header.host_end - header.host);
    TEST_ASSERT_EQUAL_STRING_LEN("127.0.0.1", header.host,
                                 header.host_end - header.host);

    TEST_ASSERT_EQUAL(port_len, header.port_end - header.port);
    TEST_ASSERT_EQUAL_STRING_LEN("8080", header.port,
                                 header.port_end - header.port);

    TEST_PASS_MESSAGE("Loopback address host test passed");
}

TEST(RX_REQUEST_HOST_HEADER, BroadcastAddressHostTest)
{
    const char *buffer    = "0.0.0.0:8080";
    const size_t addr_len = strlen("0.0.0.0");
    const size_t port_len = strlen("8080");

    int result =
        rx_request_process_header_host(&header, buffer, strlen(buffer));

    TEST_ASSERT_EQUAL_INT(RX_OK, result);
    TEST_ASSERT_EQUAL_INT(RX_REQUEST_HEADER_HOST_RESULT_OK, header.result);

    TEST_ASSERT_EQUAL(addr_len, header.host_end - header.host);
    TEST_ASSERT_EQUAL_STRING_LEN("0.0.0.0", header.host,
                                 header.host_end - header.host);

    TEST_ASSERT_EQUAL(port_len, header.port_end - header.port);
    TEST_ASSERT_EQUAL_STRING_LEN("8080", header.port,
                                 header.port_end - header.port);

    TEST_PASS_MESSAGE("Broadcast address host test passed");
}

TEST(RX_REQUEST_HOST_HEADER, SimpleInvalidAddressHostTest)
{
    const char *buffer = "172.10.0.1";
    int result         = rx_request_process_header_host(&header, buffer, 10);

    TEST_ASSERT_EQUAL_INT(RX_ERROR, result);
    TEST_ASSERT_EQUAL_INT(RX_REQUEST_HEADER_HOST_RESULT_INVALID, header.result);

    TEST_PASS_MESSAGE("Simple invalid address host test passed");
}

TEST(RX_REQUEST_HOST_HEADER, LongInvalidAddressHostTest)
{
    const char *buffer = "80.152.100.1:8080";
    int result =
        rx_request_process_header_host(&header, buffer, strlen(buffer));

    TEST_ASSERT_EQUAL_INT(RX_ERROR, result);
    TEST_ASSERT_EQUAL_INT(RX_REQUEST_HEADER_HOST_RESULT_INVALID, header.result);

    TEST_PASS_MESSAGE("Long invalid address host test passed");
}

TEST(RX_REQUEST_HOST_HEADER, LocalWithWrongPortHostTest)
{
    const char *buffer    = "localhost:8000";
    const size_t addr_len = strlen("localhost");
    const size_t port_len = strlen("8000");

    int result =
        rx_request_process_header_host(&header, buffer, strlen(buffer));

    TEST_ASSERT_EQUAL_INT(RX_ERROR, result);
    TEST_ASSERT_EQUAL_INT(RX_REQUEST_HEADER_HOST_RESULT_UNSUPPORTED,
                          header.result);

    TEST_ASSERT_EQUAL(addr_len, header.host_end - header.host);
    TEST_ASSERT_EQUAL_STRING_LEN("localhost", header.host,
                                 header.host_end - header.host);

    TEST_ASSERT_EQUAL(port_len, header.port_end - header.port);
    TEST_ASSERT_EQUAL_STRING_LEN("8000", header.port,
                                 header.port_end - header.port);

    TEST_PASS_MESSAGE("Local with wrong port host test passed");
}

TEST(RX_REQUEST_HOST_HEADER, LoopbackWithWrongPortHostTest)
{
    const char *buffer    = "127.0.0.1:5500";
    const size_t addr_len = strlen("127.0.0.1");
    const size_t port_len = strlen("5500");

    int result =
        rx_request_process_header_host(&header, buffer, strlen(buffer));

    TEST_ASSERT_EQUAL_INT(RX_ERROR, result);
    TEST_ASSERT_EQUAL_INT(RX_REQUEST_HEADER_HOST_RESULT_UNSUPPORTED,
                          header.result);

    TEST_ASSERT_EQUAL(addr_len, header.host_end - header.host);
    TEST_ASSERT_EQUAL_STRING_LEN("127.0.0.1", header.host,
                                 header.host_end - header.host);

    TEST_ASSERT_EQUAL(port_len, header.port_end - header.port);
    TEST_ASSERT_EQUAL_STRING_LEN("5500", header.port,
                                 header.port_end - header.port);

    TEST_PASS_MESSAGE("Loopback with wrong port host test passed");
}

TEST(RX_REQUEST_HOST_HEADER, BroadcastWithWrongPortHostTest)
{
    const char *buffer    = "0.0.0.0:9999";
    const size_t addr_len = strlen("0.0.0.0");
    const size_t port_len = strlen("9999");

    int result =
        rx_request_process_header_host(&header, buffer, strlen(buffer));

    TEST_ASSERT_EQUAL_INT(RX_ERROR, result);
    TEST_ASSERT_EQUAL_INT(RX_REQUEST_HEADER_HOST_RESULT_UNSUPPORTED,
                          header.result);

    TEST_ASSERT_EQUAL(addr_len, header.host_end - header.host);
    TEST_ASSERT_EQUAL_STRING_LEN("0.0.0.0", header.host,
                                 header.host_end - header.host);

    TEST_ASSERT_EQUAL(port_len, header.port_end - header.port);
    TEST_ASSERT_EQUAL_STRING_LEN("9999", header.port,
                                 header.port_end - header.port);

    TEST_PASS_MESSAGE("Broadcast with wrong port host test passed");
}

TEST_GROUP_RUNNER(RX_REQUEST_HOST_HEADER)
{
    rx_memset_header_host(&header);

    RUN_TEST_CASE(RX_REQUEST_HOST_HEADER, EmptyBufferHostHeaderTest);
    RUN_TEST_CASE(RX_REQUEST_HOST_HEADER, NullBufferHostHeaderTest);
    RUN_TEST_CASE(RX_REQUEST_HOST_HEADER, ColonAtFirstHostTest);
    RUN_TEST_CASE(RX_REQUEST_HOST_HEADER, MissingPortValueHostTest);
    RUN_TEST_CASE(RX_REQUEST_HOST_HEADER, SimpleValidHostTest);
    RUN_TEST_CASE(RX_REQUEST_HOST_HEADER, LongValidHostTest);
    RUN_TEST_CASE(RX_REQUEST_HOST_HEADER, LoopbackAddressHostTest);
    RUN_TEST_CASE(RX_REQUEST_HOST_HEADER, BroadcastAddressHostTest);
    RUN_TEST_CASE(RX_REQUEST_HOST_HEADER, SimpleInvalidAddressHostTest);
    RUN_TEST_CASE(RX_REQUEST_HOST_HEADER, LongInvalidAddressHostTest);
    RUN_TEST_CASE(RX_REQUEST_HOST_HEADER, LocalWithWrongPortHostTest);
    RUN_TEST_CASE(RX_REQUEST_HOST_HEADER, LoopbackWithWrongPortHostTest);
    RUN_TEST_CASE(RX_REQUEST_HOST_HEADER, BroadcastWithWrongPortHostTest);
}
