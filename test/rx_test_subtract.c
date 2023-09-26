#include <unity/unity.h>
#include <unity/unity_fixture.h>

TEST_GROUP(RX_SUBTRACT);

TEST_SETUP(RX_SUBTRACT)
{
}

TEST_TEAR_DOWN(RX_SUBTRACT)
{
}

TEST(RX_SUBTRACT, IgnoredTest)
{
    TEST_IGNORE_MESSAGE("This Test Was Ignored On Purpose");
}

TEST(RX_SUBTRACT, AnotherIgnoredTest)
{
    TEST_IGNORE_MESSAGE("These Can Be Useful For Leaving Yourself Notes On "
                        "What You Need To Do Yet");
}

TEST(RX_SUBTRACT, ThisFunctionHasNotBeenTested_NeedsToBeImplemented)
{
    TEST_IGNORE(); // Like This
}

TEST_GROUP_RUNNER(RX_SUBTRACT)
{
    RUN_TEST_CASE(RX_SUBTRACT, IgnoredTest);
    RUN_TEST_CASE(RX_SUBTRACT, AnotherIgnoredTest);
    RUN_TEST_CASE(RX_SUBTRACT,
                  ThisFunctionHasNotBeenTested_NeedsToBeImplemented);
}
