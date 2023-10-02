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

struct rx_qlist qlist;
const char *values[] = {
    "test-first",          "test-equal",          "test-greater-than-3",
    "test-greater-than-2", "test-greater-than-1", "test-multiple-0",
    "test-multiple-1",     "test-multiple-2",     "test-less-than-1",
    "test-less-than-2",    "test-less-than-3",    "test-zero",
};

const double qvalues[] = {
    1.0f, 1.0f, 0.9f, 0.8f, 0.7f, 0.6f, 0.5f, 0.4f, 0.3f, 0.2f, 0.1f, 0.0f,
};

TEST_GROUP(RX_QLIST);

TEST_SETUP(RX_QLIST)
{
}

TEST_TEAR_DOWN(RX_QLIST)
{
}

TEST(RX_QLIST, InitializeQListTest)
{
    int ret = rx_qlist_create(&qlist);

    TEST_ASSERT_EQUAL_INT(RX_OK, ret);
    TEST_ASSERT_EQUAL_INT(0, qlist.size);
    TEST_ASSERT_NOT_NULL(qlist.head);
    TEST_ASSERT_NULL(qlist.tail);

    TEST_PASS_MESSAGE("Initialize QList test passed");
}

TEST(RX_QLIST, AddFirstItemQListTest)
{
    const char *value = "test-first";
    size_t value_len  = strlen(value);

    int ret = rx_qlist_add(&qlist, value, value_len, 1.0f);

    TEST_ASSERT_EQUAL_INT(RX_OK, ret);
    TEST_ASSERT_EQUAL_INT(1, qlist.size);
    TEST_ASSERT_NOT_NULL(qlist.head);
    TEST_ASSERT_NOT_NULL(qlist.tail);

    TEST_PASS_MESSAGE("Add QList test passed");
}

TEST(RX_QLIST, AddEqualQValueQListTest)
{
    const char *value = "test-equal";
    size_t value_len  = strlen(value);

    int ret = rx_qlist_add(&qlist, value, value_len, 1.0f);

    TEST_ASSERT_EQUAL_INT(RX_OK, ret);
    TEST_ASSERT_EQUAL_INT(2, qlist.size);
    TEST_ASSERT_NOT_NULL(qlist.head);
    TEST_ASSERT_NOT_NULL(qlist.tail);

    TEST_PASS_MESSAGE("Add QList test passed");
}

TEST(RX_QLIST, AddLessThanQListTest1)
{
    const char *value = "test-less-than-1";
    size_t value_len  = strlen(value);

    int ret = rx_qlist_add(&qlist, value, value_len, 0.3f);

    TEST_ASSERT_EQUAL_INT(RX_OK, ret);
    TEST_ASSERT_EQUAL_INT(3, qlist.size);
    TEST_ASSERT_NOT_NULL(qlist.head);
    TEST_ASSERT_NOT_NULL(qlist.tail);

    TEST_PASS_MESSAGE("Add QList test passed");
}

TEST(RX_QLIST, AddLessThanQListTest2)
{
    const char *value = "test-less-than-2";
    size_t value_len  = strlen(value);

    int ret = rx_qlist_add(&qlist, value, value_len, 0.2f);

    TEST_ASSERT_EQUAL_INT(RX_OK, ret);
    TEST_ASSERT_EQUAL_INT(4, qlist.size);
    TEST_ASSERT_NOT_NULL(qlist.head);
    TEST_ASSERT_NOT_NULL(qlist.tail);

    TEST_PASS_MESSAGE("Add QList test passed");
}

TEST(RX_QLIST, AddLessThanQListTest3)
{
    const char *value = "test-less-than-3";
    size_t value_len  = strlen(value);

    int ret = rx_qlist_add(&qlist, value, value_len, 0.1f);

    TEST_ASSERT_EQUAL_INT(RX_OK, ret);
    TEST_ASSERT_EQUAL_INT(5, qlist.size);
    TEST_ASSERT_NOT_NULL(qlist.head);
    TEST_ASSERT_NOT_NULL(qlist.tail);

    TEST_PASS_MESSAGE("Add QList test passed");
}

TEST(RX_QLIST, AddZeroQListTest)
{
    const char *value = "test-zero";
    size_t value_len  = strlen(value);

    int ret = rx_qlist_add(&qlist, value, value_len, 0.0f);

    TEST_ASSERT_EQUAL_INT(RX_OK, ret);
    TEST_ASSERT_EQUAL_INT(6, qlist.size);
    TEST_ASSERT_NOT_NULL(qlist.head);
    TEST_ASSERT_NOT_NULL(qlist.tail);

    TEST_PASS_MESSAGE("Add QList test passed");
}

TEST(RX_QLIST, AddGreaterThanQListTest1)
{
    const char *value = "test-greater-than-1";
    size_t value_len  = strlen(value);

    int ret = rx_qlist_add(&qlist, value, value_len, 0.7f);

    TEST_ASSERT_EQUAL_INT(RX_OK, ret);
    TEST_ASSERT_EQUAL_INT(7, qlist.size);
    TEST_ASSERT_NOT_NULL(qlist.head);
    TEST_ASSERT_NOT_NULL(qlist.tail);

    TEST_PASS_MESSAGE("Add QList test passed");
}

TEST(RX_QLIST, AddGreaterThanQListTest2)
{
    const char *value = "test-greater-than-2";
    size_t value_len  = strlen(value);

    int ret = rx_qlist_add(&qlist, value, value_len, 0.8f);

    TEST_ASSERT_EQUAL_INT(RX_OK, ret);
    TEST_ASSERT_EQUAL_INT(8, qlist.size);
    TEST_ASSERT_NOT_NULL(qlist.head);
    TEST_ASSERT_NOT_NULL(qlist.tail);

    TEST_PASS_MESSAGE("Add QList test passed");
}

TEST(RX_QLIST, AddGreaterThanQListTest3)
{
    const char *value = "test-greater-than-3";
    size_t value_len  = strlen(value);

    int ret = rx_qlist_add(&qlist, value, value_len, 0.9f);

    TEST_ASSERT_EQUAL_INT(RX_OK, ret);
    TEST_ASSERT_EQUAL_INT(9, qlist.size);
    TEST_ASSERT_NOT_NULL(qlist.head);
    TEST_ASSERT_NOT_NULL(qlist.tail);

    TEST_PASS_MESSAGE("Add QList test passed");
}

TEST(RX_QLIST, AddMultipleQListTest)
{
    for (size_t i = 0; i < 3; i++)
    {
        char buf[64];
        int n = snprintf(buf, sizeof(buf), "test-multiple-%zu", i);

        int ret = rx_qlist_add(&qlist, buf, (size_t)n, 0.6f - (float)i / 10.0f);

        TEST_ASSERT_EQUAL_INT(RX_OK, ret);
        TEST_ASSERT_EQUAL_INT(10 + i, qlist.size);
        TEST_ASSERT_NOT_NULL(qlist.head);
        TEST_ASSERT_NOT_NULL(qlist.tail);
    }

    TEST_PASS_MESSAGE("Add QList test passed");
}

TEST(RX_QLIST, RetrieveQListTest)
{
    struct rx_qlist_node *node = qlist.head->next;
    size_t i                   = 0;

    TEST_ASSERT_EQUAL_INT(12, qlist.size);

    while (node != NULL)
    {
        TEST_ASSERT_EQUAL_STRING(values[i], node->value);
        TEST_ASSERT_EQUAL_FLOAT(qvalues[i], node->weight);

        i++;
        node = node->next;
    }

    TEST_PASS_MESSAGE("Retrieve QList test passed");
}

TEST(RX_QLIST, ReversedRetrieveQListTest)
{
    struct rx_qlist_node *node = qlist.tail;
    size_t i                   = 11;

    TEST_ASSERT_EQUAL_INT(12, qlist.size);

    while (node != qlist.head)
    {
        TEST_ASSERT_EQUAL_STRING(values[i], node->value);
        TEST_ASSERT_EQUAL_FLOAT(qvalues[i], node->weight);

        i--;
        node = node->prev;
    }

    TEST_PASS_MESSAGE("Reversed retrieve QList test passed");
}

TEST(RX_QLIST, DestroyQListTest)
{
    rx_qlist_destroy(&qlist);

    TEST_ASSERT_EQUAL_INT(0, qlist.size);
    TEST_ASSERT_NULL(qlist.head);
    TEST_ASSERT_NULL(qlist.tail);

    TEST_PASS_MESSAGE("Destroy QList test passed");
}

TEST_GROUP_RUNNER(RX_QLIST)
{
    RUN_TEST_CASE(RX_QLIST, InitializeQListTest);
    RUN_TEST_CASE(RX_QLIST, AddFirstItemQListTest);
    RUN_TEST_CASE(RX_QLIST, AddEqualQValueQListTest);
    RUN_TEST_CASE(RX_QLIST, AddLessThanQListTest1);
    RUN_TEST_CASE(RX_QLIST, AddLessThanQListTest2);
    RUN_TEST_CASE(RX_QLIST, AddLessThanQListTest3);
    RUN_TEST_CASE(RX_QLIST, AddZeroQListTest);
    RUN_TEST_CASE(RX_QLIST, AddGreaterThanQListTest1);
    RUN_TEST_CASE(RX_QLIST, AddGreaterThanQListTest2);
    RUN_TEST_CASE(RX_QLIST, AddGreaterThanQListTest3);
    RUN_TEST_CASE(RX_QLIST, AddMultipleQListTest);
    RUN_TEST_CASE(RX_QLIST, RetrieveQListTest);
    RUN_TEST_CASE(RX_QLIST, ReversedRetrieveQListTest);
    RUN_TEST_CASE(RX_QLIST, DestroyQListTest);
}
