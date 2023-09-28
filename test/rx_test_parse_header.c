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

#if defined(RX_DEBUG)
#undef RX_DEBUG
#endif

TEST_GROUP(RX_REQUEST_HEADER);

TEST_SETUP(RX_REQUEST_HEADER)
{
}

TEST_TEAR_DOWN(RX_REQUEST_HEADER)
{
}

TEST(RX_REQUEST_HEADER, EmptyBufferHeaderTest)
{
    char *buffer = "";
    const char *key, *key_end, *value, *value_end;

    key = key_end = value = value_end = NULL;

    int result =
        rx_request_parse_header(buffer, 0, &key, &key_end, &value, &value_end);

    TEST_ASSERT_EQUAL_INT(RX_ERROR, result);
    TEST_ASSERT_NULL(key);
    TEST_ASSERT_NULL(key_end);
    TEST_ASSERT_NULL(value);
    TEST_ASSERT_NULL(value_end);

    TEST_PASS_MESSAGE("Empty buffer header test passed");
}

TEST(RX_REQUEST_HEADER, NullBufferHeaderTest)
{
    char *buffer = NULL;
    const char *key, *key_end, *value, *value_end;

    key = key_end = value = value_end = NULL;

    int result =
        rx_request_parse_header(buffer, 0, &key, &key_end, &value, &value_end);

    TEST_ASSERT_EQUAL_INT(RX_ERROR, result);
    TEST_ASSERT_NULL(key);
    TEST_ASSERT_NULL(key_end);
    TEST_ASSERT_NULL(value);
    TEST_ASSERT_NULL(value_end);

    TEST_PASS_MESSAGE("Null buffer header test passed");
}

TEST(RX_REQUEST_HEADER, NullPointersHeaderTest)
{
    char *buffer = "Host: localhost:8080";

    int result =
        rx_request_parse_header(buffer, strlen(buffer), NULL, NULL, NULL, NULL);

    TEST_ASSERT_EQUAL_INT(RX_ERROR, result);

    TEST_PASS_MESSAGE("Null pointers header test passed");
}

TEST(RX_REQUEST_HEADER, MissingColonHeaderTest)
{
    const char *buffer = "Host localhost";
    const char *key, *key_end, *value, *value_end;

    key = key_end = value = value_end = NULL;

    int result = rx_request_parse_header(buffer, strlen(buffer), &key, &key_end,
                                         &value, &value_end);

    TEST_ASSERT_EQUAL_INT(RX_ERROR, result);
    TEST_ASSERT_NULL(key);
    TEST_ASSERT_NULL(key_end);
    TEST_ASSERT_NULL(value);
    TEST_ASSERT_NULL(value_end);

    TEST_PASS_MESSAGE("Missing colon header test passed");
}

TEST(RX_REQUEST_HEADER, MissingKeyHeaderTest)
{
    const char *buffer = ": localhost";
    const char *key, *key_end, *value, *value_end;

    key = key_end = value = value_end = NULL;

    int result = rx_request_parse_header(buffer, strlen(buffer), &key, &key_end,
                                         &value, &value_end);

    TEST_ASSERT_EQUAL_INT(RX_ERROR, result);
    TEST_ASSERT_NULL(key);
    TEST_ASSERT_NULL(key_end);
    TEST_ASSERT_NULL(value);
    TEST_ASSERT_NULL(value_end);

    TEST_PASS_MESSAGE("Missing key header test passed");
}

TEST(RX_REQUEST_HEADER, MissingValueHeaderTest)
{
    const char *buffer = "Host:";
    const char *key, *key_end, *value, *value_end;

    key = key_end = value = value_end = NULL;

    int result = rx_request_parse_header(buffer, strlen(buffer), &key, &key_end,
                                         &value, &value_end);

    TEST_ASSERT_EQUAL_INT(RX_ERROR, result);
    TEST_ASSERT_NULL(key);
    TEST_ASSERT_NULL(key_end);
    TEST_ASSERT_NULL(value);
    TEST_ASSERT_NULL(value_end);

    TEST_PASS_MESSAGE("Missing value header test passed");
}

TEST(RX_REQUEST_HEADER, ValidHeaderTest)
{
    const char *buffer = "Host: localhost:8080";
    const char *key, *key_end, *value, *value_end;

    key = key_end = value = value_end = NULL;

    int result = rx_request_parse_header(buffer, strlen(buffer), &key, &key_end,
                                         &value, &value_end);

    TEST_ASSERT_EQUAL_INT(RX_OK, result);
    TEST_ASSERT_EQUAL_STRING_LEN("Host", key, key_end - key);
    TEST_ASSERT_EQUAL_STRING_LEN("localhost:8080", value, value_end - value);

    TEST_PASS_MESSAGE("Valid header test passed");
}

TEST_GROUP_RUNNER(RX_REQUEST_HEADER)
{
    RUN_TEST_CASE(RX_REQUEST_HEADER, EmptyBufferHeaderTest);
    RUN_TEST_CASE(RX_REQUEST_HEADER, NullBufferHeaderTest);
    RUN_TEST_CASE(RX_REQUEST_HEADER, NullPointersHeaderTest);
    RUN_TEST_CASE(RX_REQUEST_HEADER, MissingColonHeaderTest);
    RUN_TEST_CASE(RX_REQUEST_HEADER, MissingKeyHeaderTest);
    RUN_TEST_CASE(RX_REQUEST_HEADER, MissingValueHeaderTest);
    RUN_TEST_CASE(RX_REQUEST_HEADER, ValidHeaderTest);
}
