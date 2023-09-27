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

struct rx_request_version version;

static void
rx_memset_version(struct rx_request_version *version)
{
    version->result = RX_REQUEST_VERSION_RESULT_NONE;
    version->major  = 0;
    version->minor  = 0;
}

TEST_GROUP(RX_REQUEST_VERSION);

TEST_SETUP(RX_REQUEST_VERSION)
{
    rx_memset_version(&version);
}

TEST_TEAR_DOWN(RX_REQUEST_VERSION)
{
}

TEST(RX_REQUEST_VERSION, EmptyBufferVersionTest)
{
    const char *buffer = "";
    int result         = rx_request_process_version(&version, buffer, 0);

    TEST_ASSERT_EQUAL_INT(RX_ERROR, result);
    TEST_ASSERT_EQUAL_INT(RX_REQUEST_VERSION_RESULT_INVALID, version.result);

    TEST_PASS_MESSAGE("Empty buffer version test passed");
}

TEST(RX_REQUEST_VERSION, NullBufferVersionTest)
{
    const char *buffer = NULL;
    int result         = rx_request_process_version(&version, buffer, 0);

    TEST_ASSERT_EQUAL_INT(RX_ERROR, result);
    TEST_ASSERT_EQUAL_INT(RX_REQUEST_VERSION_RESULT_INVALID, version.result);

    TEST_PASS_MESSAGE("Null buffer version test passed");
}

TEST(RX_REQUEST_VERSION, WrongProtocolVersionTest)
{
    const char *buffer = "FTP/1.0";
    int result = rx_request_process_version(&version, buffer, strlen(buffer));

    TEST_ASSERT_EQUAL_INT(RX_ERROR, result);
    TEST_ASSERT_EQUAL_INT(RX_REQUEST_VERSION_RESULT_INVALID, version.result);

    TEST_PASS_MESSAGE("Wrong protocol version test passed");
}

TEST(RX_REQUEST_VERSION, MissingSlashVersionTest)
{
    const char *buffer = "HTTP?1.0";
    int result = rx_request_process_version(&version, buffer, strlen(buffer));

    TEST_ASSERT_EQUAL_INT(RX_ERROR, result);
    TEST_ASSERT_EQUAL_INT(RX_REQUEST_VERSION_RESULT_INVALID, version.result);

    TEST_PASS_MESSAGE("Missing slash version test passed");
}

TEST(RX_REQUEST_VERSION, MissingDotVersionTest)
{
    const char *buffer = "HTTP/1,0";
    int result = rx_request_process_version(&version, buffer, strlen(buffer));

    TEST_ASSERT_EQUAL_INT(RX_ERROR, result);
    TEST_ASSERT_EQUAL_INT(RX_REQUEST_VERSION_RESULT_INVALID, version.result);

    TEST_PASS_MESSAGE("Missing dot version test passed");
}

TEST(RX_REQUEST_VERSION, MissingMajorVersionTest)
{
    const char *buffer = "HTTP/.1";
    int result = rx_request_process_version(&version, buffer, strlen(buffer));

    TEST_ASSERT_EQUAL_INT(RX_ERROR, result);
    TEST_ASSERT_EQUAL_INT(RX_REQUEST_VERSION_RESULT_INVALID, version.result);

    TEST_PASS_MESSAGE("Missing major version test passed");
}

TEST(RX_REQUEST_VERSION, MissingMinorVersionTest)
{
    const char *buffer = "HTTP/1.";
    int result = rx_request_process_version(&version, buffer, strlen(buffer));

    TEST_ASSERT_EQUAL_INT(RX_ERROR, result);
    TEST_ASSERT_EQUAL_INT(RX_REQUEST_VERSION_RESULT_INVALID, version.result);

    TEST_PASS_MESSAGE("Missing minor version test passed");
}

TEST(RX_REQUEST_VERSION, InvalidMajorVersionTest)
{
    const char *buffer = "HTTP/3a.0";
    int result = rx_request_process_version(&version, buffer, strlen(buffer));

    TEST_ASSERT_EQUAL_INT(RX_ERROR, result);
    TEST_ASSERT_EQUAL_INT(RX_REQUEST_VERSION_RESULT_INVALID, version.result);

    TEST_PASS_MESSAGE("Invalid major version test passed");
}

TEST(RX_REQUEST_VERSION, InvalidMinorVersionTest)
{
    const char *buffer = "HTTP/1.3a";
    int result = rx_request_process_version(&version, buffer, strlen(buffer));

    TEST_ASSERT_EQUAL_INT(RX_ERROR, result);
    TEST_ASSERT_EQUAL_INT(RX_REQUEST_VERSION_RESULT_INVALID, version.result);

    TEST_PASS_MESSAGE("Invalid minor version test passed");
}

TEST(RX_REQUEST_VERSION, UnsupportedVersionTest)
{
    const char *buffer = "HTTP/1.0";
    int result = rx_request_process_version(&version, buffer, strlen(buffer));

    TEST_ASSERT_EQUAL_INT(RX_ERROR, result);
    TEST_ASSERT_EQUAL_INT(RX_REQUEST_VERSION_RESULT_UNSUPPORTED,
                          version.result);

    TEST_PASS_MESSAGE("Unsupported version test passed");
}

TEST(RX_REQUEST_VERSION, SupportedVersionTest)
{
    const char *buffer = "HTTP/1.1";
    int result = rx_request_process_version(&version, buffer, strlen(buffer));

    TEST_ASSERT_EQUAL_INT(RX_OK, result);
    TEST_ASSERT_EQUAL_INT(RX_REQUEST_VERSION_RESULT_OK, version.result);
    TEST_ASSERT_EQUAL_INT(1, version.major);
    TEST_ASSERT_EQUAL_INT(1, version.minor);

    TEST_PASS_MESSAGE("Supported version test passed");
}

TEST_GROUP_RUNNER(RX_REQUEST_VERSION)
{
    RUN_TEST_CASE(RX_REQUEST_VERSION, EmptyBufferVersionTest);
    RUN_TEST_CASE(RX_REQUEST_VERSION, NullBufferVersionTest);
    RUN_TEST_CASE(RX_REQUEST_VERSION, WrongProtocolVersionTest);
    RUN_TEST_CASE(RX_REQUEST_VERSION, MissingSlashVersionTest);
    RUN_TEST_CASE(RX_REQUEST_VERSION, MissingDotVersionTest);
    RUN_TEST_CASE(RX_REQUEST_VERSION, MissingMajorVersionTest);
    RUN_TEST_CASE(RX_REQUEST_VERSION, MissingMinorVersionTest);
    RUN_TEST_CASE(RX_REQUEST_VERSION, InvalidMajorVersionTest);
    RUN_TEST_CASE(RX_REQUEST_VERSION, InvalidMinorVersionTest);
    RUN_TEST_CASE(RX_REQUEST_VERSION, UnsupportedVersionTest);
    RUN_TEST_CASE(RX_REQUEST_VERSION, SupportedVersionTest);
}
