#include <gtest/gtest.h>
#include <iostream>
using namespace std;

int abs(int x)
{
    return x > 0 ? x : -x;
}

int add(int a, int b)
{
    return a + b;
}

TEST(abs_test, abs)
{
    EXPECT_EQ(abs(-5), 5);
    // ASSERT_TRUE(abs(-1) == 2) << "FAILED: EXPECT: 1, but given 2\n";
}
TEST(add_test, add)
{
    // ASSERT_TRUE(add(2, 3) == 4) << "FAILED: EXPECT: 5, but given 4\n";
    ASSERT_TRUE(add(2, 2) == 4);
    EXPECT_EQ(add(2, 3), 5);
}

int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv); // 初始化 GoogleTest，将命令行参数传递给gtest

    return RUN_ALL_TESTS(); // 执行所有的 test case
}
// g++ .\src\gtest_demo.cpp -o out\gtest_out.exe -I include\googletest -L include\googletest\lib -lgtest -std=c++17