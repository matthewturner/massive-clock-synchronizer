#include <unity.h>
#include <stdio.h>

void setUp(void) {}

void tearDown(void) {}

void test_true(void)
{
    TEST_ASSERT_TRUE_MESSAGE(true, "This is true");
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();
    RUN_TEST(test_true);
    UNITY_END();

    return 0;
}