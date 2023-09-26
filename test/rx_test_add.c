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
