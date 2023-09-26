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

rx_request_method_t method;

TEST_GROUP(RX_REQUEST_METHOD);

TEST_SETUP(RX_REQUEST_METHOD)
{
    method = RX_REQUEST_METHOD_INVALID;
}

TEST_TEAR_DOWN(RX_REQUEST_METHOD)
{
}

TEST(RX_REQUEST_METHOD, GetMethodTest)
{
    const char *buffer = "GET";
    int result         = rx_request_proccess_method(&method, buffer);

    TEST_ASSERT_EQUAL_INT(RX_OK, result);
    TEST_ASSERT_EQUAL_INT(RX_REQUEST_METHOD_GET, method);

    TEST_PASS_MESSAGE("GET method test passed");
}

TEST(RX_REQUEST_METHOD, PostMethodTest)
{
    const char *buffer = "POST";
    int result         = rx_request_proccess_method(&method, buffer);

    TEST_ASSERT_EQUAL_INT(RX_OK, result);
    TEST_ASSERT_EQUAL_INT(RX_REQUEST_METHOD_POST, method);

    TEST_PASS_MESSAGE("POST method test passed");
}

TEST(RX_REQUEST_METHOD, PutMethodTest)
{
    const char *buffer = "PUT";
    int result         = rx_request_proccess_method(&method, buffer);

    TEST_ASSERT_EQUAL_INT(RX_OK, result);
    TEST_ASSERT_EQUAL_INT(RX_REQUEST_METHOD_PUT, method);

    TEST_PASS_MESSAGE("PUT method test passed");
}

TEST(RX_REQUEST_METHOD, HeadMethodTest)
{
    const char *buffer = "HEAD";
    int result         = rx_request_proccess_method(&method, buffer);

    TEST_ASSERT_EQUAL_INT(RX_OK, result);
    TEST_ASSERT_EQUAL_INT(RX_REQUEST_METHOD_HEAD, method);

    TEST_PASS_MESSAGE("HEAD method test passed");
}

TEST(RX_REQUEST_METHOD, DeleteMethodTest)
{
    const char *buffer = "DELETE";
    int result         = rx_request_proccess_method(&method, buffer);

    TEST_ASSERT_EQUAL_INT(RX_OK, result);
    TEST_ASSERT_EQUAL_INT(RX_REQUEST_METHOD_DELETE, method);

    TEST_PASS_MESSAGE("DELETE method test passed");
}

TEST(RX_REQUEST_METHOD, NullMethodTest)
{
    const char *buffer = NULL;
    int result         = rx_request_proccess_method(&method, buffer);

    TEST_ASSERT_EQUAL_INT(RX_ERROR, result);
    TEST_ASSERT_EQUAL_INT(RX_REQUEST_METHOD_INVALID, method);

    TEST_PASS_MESSAGE("NULL method test passed");
}

TEST(RX_REQUEST_METHOD, EmptyMethodTest)
{
    const char *buffer = "";
    int result         = rx_request_proccess_method(&method, buffer);

    TEST_ASSERT_EQUAL_INT(RX_ERROR, result);
    TEST_ASSERT_EQUAL_INT(RX_REQUEST_METHOD_INVALID, method);

    TEST_PASS_MESSAGE("Empty method test passed");
}

TEST(RX_REQUEST_METHOD, UnsupportedMethodTest)
{
    const char *buffer = "CONNECT";
    int result         = rx_request_proccess_method(&method, buffer);

    TEST_ASSERT_EQUAL_INT(RX_OK, result);
    TEST_ASSERT_EQUAL_INT(RX_REQUEST_METHOD_INVALID, method);

    TEST_PASS_MESSAGE("Unsupported method test passed");
}

TEST_GROUP_RUNNER(RX_REQUEST_METHOD)
{
    RUN_TEST_CASE(RX_REQUEST_METHOD, GetMethodTest);
    RUN_TEST_CASE(RX_REQUEST_METHOD, PostMethodTest);
    RUN_TEST_CASE(RX_REQUEST_METHOD, PutMethodTest);
    RUN_TEST_CASE(RX_REQUEST_METHOD, HeadMethodTest);
    RUN_TEST_CASE(RX_REQUEST_METHOD, DeleteMethodTest);
    RUN_TEST_CASE(RX_REQUEST_METHOD, NullMethodTest);
    RUN_TEST_CASE(RX_REQUEST_METHOD, EmptyMethodTest);
    RUN_TEST_CASE(RX_REQUEST_METHOD, UnsupportedMethodTest);
}
