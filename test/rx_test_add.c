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

TEST_GROUP(RX_ADD);

TEST_SETUP(RX_ADD)
{
}

TEST_TEAR_DOWN(RX_ADD)
{
}

TEST(RX_ADD, AddToEqualTest)
{
    TEST_ASSERT_EQUAL(0, 0 + 0);
    TEST_ASSERT_EQUAL(9, 5 + 4);
    TEST_ASSERT_EQUAL(1, 5 + (-4));

    TEST_PASS_MESSAGE("All add-to-equal tests passed");
}

TEST(RX_ADD, AddToGreaterTest)
{
    TEST_ASSERT_GREATER_THAN(100, 100 + 1);
    TEST_ASSERT_GREATER_THAN(10, 5 + 6);
    TEST_ASSERT_GREATER_THAN(0, 5 + (-4));

    TEST_PASS_MESSAGE("All add-to-greater tests passed");
}

TEST(RX_ADD, ThisFunctionHasNotBeenTested_NeedsToBeImplemented)
{
    TEST_IGNORE(); // Like This
}

TEST_GROUP_RUNNER(RX_ADD)
{
    RUN_TEST_CASE(RX_ADD, AddToEqualTest);
    RUN_TEST_CASE(RX_ADD, AddToGreaterTest);
    RUN_TEST_CASE(RX_ADD, ThisFunctionHasNotBeenTested_NeedsToBeImplemented);
}
