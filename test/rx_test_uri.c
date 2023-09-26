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

struct rx_request_uri uri;

static void
rx_memset_uri(struct rx_request_uri *uri)
{
    memset(uri->raw_uri, 0, sizeof(uri->raw_uri));

    uri->length           = 0;
    uri->path             = NULL;
    uri->query_string     = NULL;
    uri->path_end         = NULL;
    uri->query_string_end = NULL;

    uri->result = RX_REQUEST_URI_RESULT_NONE;
}

TEST_GROUP(RX_REQUEST_URI);

TEST_SETUP(RX_REQUEST_URI)
{
    rx_memset_uri(&uri);
}

TEST_TEAR_DOWN(RX_REQUEST_URI)
{
}

TEST(RX_REQUEST_URI, EmptyUriTest)
{
    const char *raw_uri = "";
    rx_request_process_uri(&uri, raw_uri);

    TEST_ASSERT_EQUAL_STRING("", uri.raw_uri);
    TEST_ASSERT_EQUAL(0, uri.length);
    TEST_ASSERT_EQUAL_PTR(NULL, uri.path);
    TEST_ASSERT_EQUAL_PTR(NULL, uri.query_string);
    TEST_ASSERT_EQUAL_PTR(NULL, uri.path_end);
    TEST_ASSERT_EQUAL_PTR(NULL, uri.query_string_end);

    TEST_ASSERT_EQUAL(RX_REQUEST_URI_RESULT_INVALID, uri.result);
    TEST_PASS_MESSAGE("Empty URI test passed");
}

TEST(RX_REQUEST_URI, IndexUriTest)
{
    const char *raw_uri = "/";
    rx_request_process_uri(&uri, raw_uri);

    TEST_ASSERT_EQUAL_STRING("/", uri.raw_uri);
    TEST_ASSERT_EQUAL(1, uri.length);
    TEST_ASSERT_EQUAL_STRING("/", uri.path);
    TEST_ASSERT_EQUAL_STRING("\0", uri.path_end);

    TEST_ASSERT_EQUAL_PTR(uri.raw_uri, uri.path);
    TEST_ASSERT_EQUAL_PTR(NULL, uri.query_string);
    TEST_ASSERT_EQUAL_PTR(uri.raw_uri + uri.length, uri.path_end);
    TEST_ASSERT_EQUAL_PTR(NULL, uri.query_string_end);

    TEST_ASSERT_EQUAL(RX_REQUEST_URI_RESULT_OK, uri.result);

    TEST_PASS_MESSAGE("Index URI test passed");
}

TEST(RX_REQUEST_URI, UriWithNoSlashTest)
{
    const char *raw_uri = "index.html";
    const size_t len    = strlen(raw_uri);
    rx_request_process_uri(&uri, raw_uri);

    TEST_ASSERT_EQUAL_STRING("index.html", uri.raw_uri);
    TEST_ASSERT_EQUAL(len, uri.length);
    TEST_ASSERT_EQUAL_STRING("index.html", uri.path);
    TEST_ASSERT_EQUAL_STRING("\0", uri.path_end);

    TEST_ASSERT_EQUAL_PTR(uri.raw_uri, uri.path);
    TEST_ASSERT_EQUAL_PTR(NULL, uri.query_string);
    TEST_ASSERT_EQUAL_PTR(uri.raw_uri + uri.length, uri.path_end);
    TEST_ASSERT_EQUAL_PTR(NULL, uri.query_string_end);

    TEST_ASSERT_EQUAL(RX_REQUEST_URI_RESULT_OK, uri.result);

    TEST_PASS_MESSAGE("URI with no slash test passed");
}

TEST(RX_REQUEST_URI, MultiPathUriTest)
{
    const char *raw_uri = "/questions/1024/what-is-the-difference-between"
                          "-a-uri-a-url-and-a-urn";
    const size_t len    = strlen(raw_uri);
    rx_request_process_uri(&uri, raw_uri);

    TEST_ASSERT_EQUAL_STRING(raw_uri, uri.raw_uri);
    TEST_ASSERT_EQUAL(len, uri.length);
    TEST_ASSERT_EQUAL_STRING(raw_uri, uri.path);
    TEST_ASSERT_EQUAL_STRING("\0", uri.path_end);

    TEST_ASSERT_EQUAL_PTR(uri.raw_uri, uri.path);
    TEST_ASSERT_EQUAL_PTR(NULL, uri.query_string);
    TEST_ASSERT_EQUAL_PTR(uri.raw_uri + uri.length, uri.path_end);
    TEST_ASSERT_EQUAL_PTR(NULL, uri.query_string_end);

    TEST_ASSERT_EQUAL(RX_REQUEST_URI_RESULT_OK, uri.result);

    TEST_PASS_MESSAGE("Multi-path URI test passed");
}

TEST(RX_REQUEST_URI, QueryStringUriTest)
{
    const char *raw_uri = "/index.html?foo=bar&baz=waldo";
    const size_t len    = strlen(raw_uri);
    rx_request_process_uri(&uri, raw_uri);

    TEST_ASSERT_EQUAL_STRING(raw_uri, uri.raw_uri);
    TEST_ASSERT_EQUAL(len, uri.length);

    TEST_ASSERT_EQUAL_STRING_LEN("/index.html", raw_uri, 11);
    TEST_ASSERT_EQUAL_STRING_LEN("?", uri.path_end, 1);
    TEST_ASSERT_EQUAL(11, uri.path_end - uri.path);

    TEST_ASSERT_EQUAL_STRING_LEN("foo=bar&baz=waldo", uri.query_string, 17);
    TEST_ASSERT_EQUAL_STRING_LEN("\0", uri.query_string_end, 1);
    TEST_ASSERT_EQUAL(17, uri.query_string_end - uri.query_string);

    TEST_ASSERT_EQUAL_PTR(uri.raw_uri, uri.path);
    TEST_ASSERT_EQUAL_PTR(uri.raw_uri + 12, uri.query_string);
    TEST_ASSERT_EQUAL_PTR(uri.raw_uri + 11, uri.path_end);
    TEST_ASSERT_EQUAL_PTR(uri.raw_uri + uri.length, uri.query_string_end);

    TEST_ASSERT_EQUAL(RX_REQUEST_URI_RESULT_OK, uri.result);

    TEST_PASS_MESSAGE("Query string URI test passed");
}

TEST(RX_REQUEST_URI, TooLongUriTest)
{
    const char *raw_uri = "/index.html"
                          "/01234567890123456789012345678901234567890123456789"
                          "/01234567890123456789012345678901234567890123456789"
                          "/01234567890123456789012345678901234567890123456789"
                          "/01234567890123456789012345678901234567890123456789"
                          "/01234567890123456789012345678901234567890123456789"
                          "/01234567890123456789012345678901234567890123456789"
                          "/01234567890123456789012345678901234567890123456789"
                          "/01234567890123456789012345678901234567890123456789"
                          "/01234567890123456789012345678901234567890123456789"
                          "/01234567890123456789012345678901234567890123456789"
                          "/01234567890123456789012345678901234567890123456789"
                          "/01234567890123456789012345678901234567890123456789"
                          "/01234567890123456789012345678901234567890123456789"
                          "/01234567890123456789012345678901234567890123456789"
                          "/01234567890123456789012345678901234567890123456789"
                          "/01234567890123456789012345678901234567890123456789"
                          "/01234567890123456789012345678901234567890123456789"
                          "/01234567890123456789012345678901234567890123456789"
                          "/01234567890123456789012345678901234567890123456789"
                          "/01234567890123456789012345678901234567890123456789"
                          "/01234567890123456789012345678901234567890123456789"
                          "/01234567890123456789012345678901234567890123456789"
                          "/01234567890123456789012345678901234567890123456789"
                          "/01234567890123456789012345678901234567890123456789"
                          "/01234567890123456789012345678901234567890123456789"
                          "/01234567890123456789012345678901234567890123456789"
                          "/01234567890123456789012345678901234567890123456789"
                          "/01234567890123456789012345678901234567890123456789"
                          "/01234567890123456789012345678901234567890123456789"
                          "/01234567890123456789012345678901234567890123456789"
                          "/01234567890123456789012345678901234567890123456789"
                          "/01234567890123456789012345678901234567890123456789"
                          "/01234567890123456789012345678901234567890123456789"
                          "/01234567890123456789012345678901234567890123456789"
                          "/01234567890123456789012345678901234567890123456789"
                          "/01234567890123456789012345678901234567890123456789"
                          "/01234567890123456789012345678901234567890123456789"
                          "/01234567890123456789012345678901234567890123456789"
                          "/01234567890123456789012345678901234567890123456789"
                          "/01234567890123456789012345678901234567890123456789"
                          "/01234567890123456789012345678901234567890123456789"
                          "/01234567890123456789012345678901234567890123456789"
                          "/01234567890123456789012345678901234567890123456789"
                          "/01234567890123456789012345678901234567890123456789"
                          "/01234567890123456789012345678901234567890123456789"
                          "/01234567890123456789012345678901234567890123456789";
    rx_request_process_uri(&uri, raw_uri);

    TEST_ASSERT_EQUAL_STRING("\0", uri.raw_uri);
    TEST_ASSERT_EQUAL(0, uri.length);

    TEST_ASSERT_EQUAL(RX_REQUEST_URI_RESULT_TOO_LONG, uri.result);

    TEST_PASS_MESSAGE("Too long URI test passed");
}

TEST_GROUP_RUNNER(RX_REQUEST_URI)
{
    RUN_TEST_CASE(RX_REQUEST_URI, EmptyUriTest);
    RUN_TEST_CASE(RX_REQUEST_URI, IndexUriTest);
    RUN_TEST_CASE(RX_REQUEST_URI, UriWithNoSlashTest);
    RUN_TEST_CASE(RX_REQUEST_URI, MultiPathUriTest);
    RUN_TEST_CASE(RX_REQUEST_URI, QueryStringUriTest);
    RUN_TEST_CASE(RX_REQUEST_URI, TooLongUriTest);
}
