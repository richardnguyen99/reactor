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

struct rx_header_accept_encoding ae_header;

static void
rx_memset_header_accept_encoding(
    struct rx_header_accept_encoding *accept_encoding)
{
    accept_encoding->encoding = RX_ENCODING_UNSET;
    accept_encoding->qvalue   = 0.0;
}

TEST_GROUP(RX_REQUEST_ACCEPT_ENCODING_HEADER);

TEST_SETUP(RX_REQUEST_ACCEPT_ENCODING_HEADER)
{
    rx_memset_header_accept_encoding(&ae_header);
}

TEST_TEAR_DOWN(RX_REQUEST_ACCEPT_ENCODING_HEADER)
{
    rx_memset_header_accept_encoding(&ae_header);
}

TEST(RX_REQUEST_ACCEPT_ENCODING_HEADER, EmptyBufferTest)
{
    const char *buffer = "";
    size_t buffer_size = 0;

    int ret = rx_request_process_header_accept_encoding(&ae_header, buffer,
                                                        buffer_size);

    TEST_ASSERT_EQUAL_INT(RX_OK, ret);
    TEST_ASSERT_EQUAL_INT(RX_ENCODING_IDENTITY, ae_header.encoding);
    TEST_ASSERT_EQUAL_FLOAT(1.0, ae_header.qvalue);

    TEST_PASS_MESSAGE("Empty buffer test passed.");
}

TEST(RX_REQUEST_ACCEPT_ENCODING_HEADER, NullEmptyBufferTest)
{
    const char *buffer = NULL;
    size_t buffer_size = 0;

    int ret = rx_request_process_header_accept_encoding(&ae_header, buffer,
                                                        buffer_size);

    TEST_ASSERT_EQUAL_INT(RX_OK, ret);
    TEST_ASSERT_EQUAL_INT(RX_ENCODING_IDENTITY, ae_header.encoding);
    TEST_ASSERT_EQUAL_FLOAT(1.0, ae_header.qvalue);

    TEST_PASS_MESSAGE("Null empty buffer test passed.");
}

TEST(RX_REQUEST_ACCEPT_ENCODING_HEADER, InvalidValueTest)
{
    const char *buffer = "zzip";
    size_t buffer_size = strlen(buffer);

    int ret = rx_request_process_header_accept_encoding(&ae_header, buffer,
                                                        buffer_size);

    TEST_ASSERT_EQUAL_INT(RX_OK, ret);
    TEST_ASSERT_EQUAL_INT(RX_ENCODING_IDENTITY, ae_header.encoding);
    TEST_ASSERT_EQUAL_FLOAT(1.0, ae_header.qvalue);

    TEST_PASS_MESSAGE("Invalid value test passed.");
}

TEST(RX_REQUEST_ACCEPT_ENCODING_HEADER, SingleValidValueTest)
{
    const char *buffer = "gzip";
    size_t buffer_size = strlen(buffer);

    int ret = rx_request_process_header_accept_encoding(&ae_header, buffer,
                                                        buffer_size);

    TEST_ASSERT_EQUAL_INT(RX_OK, ret);
    TEST_ASSERT_EQUAL_INT(RX_ENCODING_GZIP, ae_header.encoding);
    TEST_ASSERT_EQUAL_FLOAT(1.0, ae_header.qvalue);

    TEST_PASS_MESSAGE("Single valid value test passed.");
}

TEST(RX_REQUEST_ACCEPT_ENCODING_HEADER, DeflateValueTest)
{
    const char *buffer = "deflate";
    size_t buffer_size = strlen(buffer);

    int ret = rx_request_process_header_accept_encoding(&ae_header, buffer,
                                                        buffer_size);

    TEST_ASSERT_EQUAL_INT(RX_OK, ret);

    TEST_ASSERT_EQUAL_INT(RX_ENCODING_DEFLATE, ae_header.encoding);
    TEST_ASSERT_EQUAL_FLOAT(1.0, ae_header.qvalue);

    TEST_PASS_MESSAGE("Deflate value test passed.");
}

TEST(RX_REQUEST_ACCEPT_ENCODING_HEADER, CompressValueTest)
{
    const char *buffer = "compress";
    size_t buffer_size = strlen(buffer);

    int ret = rx_request_process_header_accept_encoding(&ae_header, buffer,
                                                        buffer_size);

    TEST_ASSERT_EQUAL_INT(RX_OK, ret);

    TEST_ASSERT_EQUAL_INT(RX_ENCODING_COMPRESS, ae_header.encoding);
    TEST_ASSERT_EQUAL_FLOAT(1.0, ae_header.qvalue);

    TEST_PASS_MESSAGE("Compress value test passed.");
}

TEST(RX_REQUEST_ACCEPT_ENCODING_HEADER, BrotliValueTest)
{
    const char *buffer = "br";
    size_t buffer_size = strlen(buffer);

    int ret = rx_request_process_header_accept_encoding(&ae_header, buffer,
                                                        buffer_size);

    TEST_ASSERT_EQUAL_INT(RX_OK, ret);

    TEST_ASSERT_EQUAL_INT(RX_ENCODING_BROTLI, ae_header.encoding);
    TEST_ASSERT_EQUAL_FLOAT(1.0, ae_header.qvalue);

    TEST_PASS_MESSAGE("Brotli value test passed.");
}

TEST(RX_REQUEST_ACCEPT_ENCODING_HEADER, AnyValueTest)
{
    const char *buffer = "*";
    size_t buffer_size = strlen(buffer);

    int ret = rx_request_process_header_accept_encoding(&ae_header, buffer,
                                                        buffer_size);

    TEST_ASSERT_EQUAL_INT(RX_OK, ret);

    TEST_ASSERT_EQUAL_INT(RX_ENCODING_ANY, ae_header.encoding);
    TEST_ASSERT_EQUAL_FLOAT(1.0, ae_header.qvalue);

    TEST_PASS_MESSAGE("Any value test passed.");
}

TEST(RX_REQUEST_ACCEPT_ENCODING_HEADER, MultipleValieValueTest)
{
    const char *buffer = "deflate, gzip";
    size_t buffer_size = strlen(buffer);

    int ret = rx_request_process_header_accept_encoding(&ae_header, buffer,
                                                        buffer_size);

    TEST_ASSERT_EQUAL_INT(RX_OK, ret);
    TEST_ASSERT_EQUAL_INT(RX_ENCODING_DEFLATE, ae_header.encoding);
    TEST_ASSERT_EQUAL_FLOAT(1.0, ae_header.qvalue);

    TEST_PASS_MESSAGE("Multiple valid value test passed.");
}

TEST(RX_REQUEST_ACCEPT_ENCODING_HEADER, MultipleWhiteSpaceValueTest)
{
    const char *buffer = "compress,   gzip, deflate";
    size_t buffer_size = strlen(buffer);

    int ret = rx_request_process_header_accept_encoding(&ae_header, buffer,
                                                        buffer_size);

    TEST_ASSERT_EQUAL_INT(RX_OK, ret);
    TEST_ASSERT_EQUAL_INT(RX_ENCODING_COMPRESS, ae_header.encoding);
    TEST_ASSERT_EQUAL_FLOAT(1.0, ae_header.qvalue);

    TEST_PASS_MESSAGE("Multiple white space value test passed.");
}

TEST(RX_REQUEST_ACCEPT_ENCODING_HEADER, QValueSingleValueTest)
{
    const char *buffer = "gzip;q=0.5";
    size_t buffer_size = strlen(buffer);

    int ret = rx_request_process_header_accept_encoding(&ae_header, buffer,
                                                        buffer_size);

    TEST_ASSERT_EQUAL_INT(RX_OK, ret);
    TEST_ASSERT_EQUAL_INT(RX_ENCODING_GZIP, ae_header.encoding);
    TEST_ASSERT_EQUAL_FLOAT(0.5, ae_header.qvalue);

    TEST_PASS_MESSAGE("QValue single value test passed.");
}

TEST(RX_REQUEST_ACCEPT_ENCODING_HEADER, DeflateWithQValueTest)
{
    const char *buffer = "deflate;q=0.7";
    size_t buffer_size = strlen(buffer);

    int ret = rx_request_process_header_accept_encoding(&ae_header, buffer,
                                                        buffer_size);

    TEST_ASSERT_EQUAL_INT(RX_OK, ret);
    TEST_ASSERT_EQUAL_INT(RX_ENCODING_DEFLATE, ae_header.encoding);
    TEST_ASSERT_EQUAL_FLOAT(0.7, ae_header.qvalue);

    TEST_PASS_MESSAGE("Deflate with QValue test passed.");
}

TEST(RX_REQUEST_ACCEPT_ENCODING_HEADER, DeflateWithQValueLessThanZeroTest)
{
    const char *buffer = "deflate;q=-0.7";
    size_t buffer_size = strlen(buffer);

    int ret = rx_request_process_header_accept_encoding(&ae_header, buffer,
                                                        buffer_size);

    // Should be invalid
    TEST_ASSERT_EQUAL_INT(RX_OK, ret);
    TEST_ASSERT_EQUAL_INT(RX_ENCODING_IDENTITY, ae_header.encoding);
    TEST_ASSERT_EQUAL_FLOAT(1.0, ae_header.qvalue);

    TEST_PASS_MESSAGE("Deflate with QValue less than zero test passed.");
}

TEST(RX_REQUEST_ACCEPT_ENCODING_HEADER, DeflateWithQValueMoreThanOneTest)
{
    const char *buffer = "deflate;q=1.7";
    size_t buffer_size = strlen(buffer);

    int ret = rx_request_process_header_accept_encoding(&ae_header, buffer,
                                                        buffer_size);
    // Should be invalid
    TEST_ASSERT_EQUAL_INT(RX_OK, ret);
    TEST_ASSERT_EQUAL_INT(RX_ENCODING_DEFLATE, ae_header.encoding);
    TEST_ASSERT_EQUAL_FLOAT(1.0, ae_header.qvalue);

    TEST_PASS_MESSAGE("Deflate with QValue more than one test passed.");
}

TEST(RX_REQUEST_ACCEPT_ENCODING_HEADER, GzipWithQValueLessThanZeroTest)
{
    const char *buffer = "gzip;q=-0.7";
    size_t buffer_size = strlen(buffer);

    int ret = rx_request_process_header_accept_encoding(&ae_header, buffer,
                                                        buffer_size);

    // Should be invalid
    TEST_ASSERT_EQUAL_INT(RX_OK, ret);
    TEST_ASSERT_EQUAL_INT(RX_ENCODING_IDENTITY, ae_header.encoding);
    TEST_ASSERT_EQUAL_FLOAT(1.0, ae_header.qvalue);

    TEST_PASS_MESSAGE("gzip with QValue less than zero test passed.");
}

TEST(RX_REQUEST_ACCEPT_ENCODING_HEADER, GzipWithQValueMoreThanOneTest)
{
    const char *buffer = "gzip;q=1.7";
    size_t buffer_size = strlen(buffer);

    int ret = rx_request_process_header_accept_encoding(&ae_header, buffer,
                                                        buffer_size);
    // Should be invalid
    TEST_ASSERT_EQUAL_INT(RX_OK, ret);
    TEST_ASSERT_EQUAL_INT(RX_ENCODING_GZIP, ae_header.encoding);
    TEST_ASSERT_EQUAL_FLOAT(1.0, ae_header.qvalue);

    TEST_PASS_MESSAGE("Gzip with QValue more than one test passed.");
}

TEST(RX_REQUEST_ACCEPT_ENCODING_HEADER, CompressWithQValueLessThanZeroTest)
{
    const char *buffer = "compress;q=-0.7";
    size_t buffer_size = strlen(buffer);

    int ret = rx_request_process_header_accept_encoding(&ae_header, buffer,
                                                        buffer_size);

    // Should be invalid
    TEST_ASSERT_EQUAL_INT(RX_OK, ret);
    TEST_ASSERT_EQUAL_INT(RX_ENCODING_IDENTITY, ae_header.encoding);
    TEST_ASSERT_EQUAL_FLOAT(1.0, ae_header.qvalue);

    TEST_PASS_MESSAGE("Compress with QValue less than zero test passed.");
}

TEST(RX_REQUEST_ACCEPT_ENCODING_HEADER, CompressWithQValueMoreThanOneTest)
{
    const char *buffer = "compress;q=1.7";
    size_t buffer_size = strlen(buffer);

    int ret = rx_request_process_header_accept_encoding(&ae_header, buffer,
                                                        buffer_size);
    // Should be invalid
    TEST_ASSERT_EQUAL_INT(RX_OK, ret);
    TEST_ASSERT_EQUAL_INT(RX_ENCODING_COMPRESS, ae_header.encoding);
    TEST_ASSERT_EQUAL_FLOAT(1.0, ae_header.qvalue);

    TEST_PASS_MESSAGE("Compress with QValue more than one test passed.");
}

TEST(RX_REQUEST_ACCEPT_ENCODING_HEADER, BrotliWithQValueLessThanZeroTest)
{
    const char *buffer = "br;q=-0.7";
    size_t buffer_size = strlen(buffer);

    int ret = rx_request_process_header_accept_encoding(&ae_header, buffer,
                                                        buffer_size);

    // Should be invalid
    TEST_ASSERT_EQUAL_INT(RX_OK, ret);
    TEST_ASSERT_EQUAL_INT(RX_ENCODING_IDENTITY, ae_header.encoding);
    TEST_ASSERT_EQUAL_FLOAT(1.0, ae_header.qvalue);

    TEST_PASS_MESSAGE("Brotli with QValue less than zero test passed.");
}

TEST(RX_REQUEST_ACCEPT_ENCODING_HEADER, BrotliWithQValueMoreThanOneTest)
{
    const char *buffer = "br;q=1.7";
    size_t buffer_size = strlen(buffer);

    int ret = rx_request_process_header_accept_encoding(&ae_header, buffer,
                                                        buffer_size);
    // Should be invalid
    TEST_ASSERT_EQUAL_INT(RX_OK, ret);
    TEST_ASSERT_EQUAL_INT(RX_ENCODING_BROTLI, ae_header.encoding);
    TEST_ASSERT_EQUAL_FLOAT(1.0, ae_header.qvalue);

    TEST_PASS_MESSAGE("Brotli with QValue more than one test passed.");
}

TEST(RX_REQUEST_ACCEPT_ENCODING_HEADER, AnyWithQValueLessThanZeroTest)
{
    const char *buffer = "*;q=-0.7";
    size_t buffer_size = strlen(buffer);

    int ret = rx_request_process_header_accept_encoding(&ae_header, buffer,
                                                        buffer_size);

    // Should be invalid
    TEST_ASSERT_EQUAL_INT(RX_OK, ret);
    TEST_ASSERT_EQUAL_INT(RX_ENCODING_IDENTITY, ae_header.encoding);
    TEST_ASSERT_EQUAL_FLOAT(1.0, ae_header.qvalue);

    TEST_PASS_MESSAGE("Any with QValue less than zero test passed.");
}

TEST(RX_REQUEST_ACCEPT_ENCODING_HEADER, AnyWithQValueMoreThanOneTest)
{
    const char *buffer = "*;q=1.7";
    size_t buffer_size = strlen(buffer);

    int ret = rx_request_process_header_accept_encoding(&ae_header, buffer,
                                                        buffer_size);
    // Should be invalid
    TEST_ASSERT_EQUAL_INT(RX_OK, ret);
    TEST_ASSERT_EQUAL_INT(RX_ENCODING_ANY, ae_header.encoding);
    TEST_ASSERT_EQUAL_FLOAT(1.0, ae_header.qvalue);

    TEST_PASS_MESSAGE("Any with QValue more than one test passed.");
}

TEST(RX_REQUEST_ACCEPT_ENCODING_HEADER, QValueMultipleValueTest)
{
    const char *buffer = "gzip;q=0.5, deflate;q=0.8";
    size_t buffer_size = strlen(buffer);

    int ret = rx_request_process_header_accept_encoding(&ae_header, buffer,
                                                        buffer_size);

    TEST_ASSERT_EQUAL_INT(RX_OK, ret);
    TEST_ASSERT_EQUAL_INT(RX_ENCODING_DEFLATE, ae_header.encoding);
    TEST_ASSERT_EQUAL_FLOAT(0.8, ae_header.qvalue);

    TEST_PASS_MESSAGE("QValue multiple value test passed.");
}

TEST(RX_REQUEST_ACCEPT_ENCODING_HEADER, Complex1Test)
{
    const char *buffer = "gzip;q=0.5, deflate;q=0.8, br;q=1.0, *;q=0.9";
    size_t buffer_size = strlen(buffer);

    int ret = rx_request_process_header_accept_encoding(&ae_header, buffer,
                                                        buffer_size);
    TEST_ASSERT_EQUAL_INT(RX_OK, ret);
    TEST_ASSERT_EQUAL_INT(RX_ENCODING_BROTLI, ae_header.encoding);
    TEST_ASSERT_EQUAL_FLOAT(1.0, ae_header.qvalue);

    TEST_PASS_MESSAGE("Complex 1 test passed.");
}

TEST(RX_REQUEST_ACCEPT_ENCODING_HEADER, Complex2Text)
{
    const char *buffer = "gzip;q=0.5, deflate;q=1.8, br;q=1.0, *;q=0.9, "
                         "compress;q=0.7";
    size_t buffer_size = strlen(buffer);

    int ret = rx_request_process_header_accept_encoding(&ae_header, buffer,
                                                        buffer_size);
    TEST_ASSERT_EQUAL_INT(RX_OK, ret);
    TEST_ASSERT_EQUAL_INT(RX_ENCODING_DEFLATE, ae_header.encoding);
    TEST_ASSERT_EQUAL_FLOAT(1.0, ae_header.qvalue);

    TEST_PASS_MESSAGE("Complex 2 test passed.");
}

TEST_GROUP_RUNNER(RX_REQUEST_ACCEPT_ENCODING_HEADER)
{
    RUN_TEST_CASE(RX_REQUEST_ACCEPT_ENCODING_HEADER, EmptyBufferTest);
    RUN_TEST_CASE(RX_REQUEST_ACCEPT_ENCODING_HEADER, NullEmptyBufferTest);
    RUN_TEST_CASE(RX_REQUEST_ACCEPT_ENCODING_HEADER, InvalidValueTest);
    RUN_TEST_CASE(RX_REQUEST_ACCEPT_ENCODING_HEADER, SingleValidValueTest);

    RUN_TEST_CASE(RX_REQUEST_ACCEPT_ENCODING_HEADER, DeflateValueTest);
    RUN_TEST_CASE(RX_REQUEST_ACCEPT_ENCODING_HEADER, CompressValueTest);
    RUN_TEST_CASE(RX_REQUEST_ACCEPT_ENCODING_HEADER, BrotliValueTest);
    RUN_TEST_CASE(RX_REQUEST_ACCEPT_ENCODING_HEADER, AnyValueTest);

    RUN_TEST_CASE(RX_REQUEST_ACCEPT_ENCODING_HEADER, MultipleValieValueTest);
    RUN_TEST_CASE(RX_REQUEST_ACCEPT_ENCODING_HEADER,
                  MultipleWhiteSpaceValueTest);

    RUN_TEST_CASE(RX_REQUEST_ACCEPT_ENCODING_HEADER, QValueSingleValueTest);
    RUN_TEST_CASE(RX_REQUEST_ACCEPT_ENCODING_HEADER, DeflateWithQValueTest);
    RUN_TEST_CASE(RX_REQUEST_ACCEPT_ENCODING_HEADER,
                  DeflateWithQValueLessThanZeroTest);
    RUN_TEST_CASE(RX_REQUEST_ACCEPT_ENCODING_HEADER,
                  DeflateWithQValueMoreThanOneTest);
    RUN_TEST_CASE(RX_REQUEST_ACCEPT_ENCODING_HEADER,
                  GzipWithQValueLessThanZeroTest);
    RUN_TEST_CASE(RX_REQUEST_ACCEPT_ENCODING_HEADER,
                  GzipWithQValueMoreThanOneTest);
    RUN_TEST_CASE(RX_REQUEST_ACCEPT_ENCODING_HEADER,
                  CompressWithQValueLessThanZeroTest);
    RUN_TEST_CASE(RX_REQUEST_ACCEPT_ENCODING_HEADER,
                  CompressWithQValueMoreThanOneTest);
    RUN_TEST_CASE(RX_REQUEST_ACCEPT_ENCODING_HEADER,
                  BrotliWithQValueLessThanZeroTest);
    RUN_TEST_CASE(RX_REQUEST_ACCEPT_ENCODING_HEADER,
                  BrotliWithQValueMoreThanOneTest);
    RUN_TEST_CASE(RX_REQUEST_ACCEPT_ENCODING_HEADER,
                  AnyWithQValueLessThanZeroTest);
    RUN_TEST_CASE(RX_REQUEST_ACCEPT_ENCODING_HEADER,
                  AnyWithQValueMoreThanOneTest);

    RUN_TEST_CASE(RX_REQUEST_ACCEPT_ENCODING_HEADER, QValueMultipleValueTest);
    RUN_TEST_CASE(RX_REQUEST_ACCEPT_ENCODING_HEADER, Complex1Test);
    RUN_TEST_CASE(RX_REQUEST_ACCEPT_ENCODING_HEADER, Complex2Text);
}
