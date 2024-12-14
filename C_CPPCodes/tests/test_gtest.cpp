#include <gtest/gtest.h>
#include <iostream>

int Hello() {

    std::cout << "hello" << std::endl;

    return 0;
}

int abs(int x)
{
    return x > 0 ? x : -x;
}

int add(int a, int b)
{
    return a + b;
}


/**
 * gtest中提供的宏分为ASSERT和EXPECT两种，
 * 其中ASSERT宏会在检查到错误的时候直接终止单元测试用例的运行（注意是单个单元测试），而EXPECT不会。
*/

// 一个单元测试模块
TEST(Hello0,  Hello) {
    EXPECT_EQ(0, Hello());
}


TEST(abs_test, abs)
{
    EXPECT_EQ(abs(-5), 5);
    ASSERT_TRUE(abs(-1) == 2) << "FAILED: EXPECT: 1, but given 2\n";
    EXPECT_EQ(abs(-12), 5);
}

TEST(add_test, add)
{
    // ASSERT_TRUE(add(2, 3) == 4) << "FAILED: EXPECT: 5, but given 4\n";
    ASSERT_TRUE(add(2, 2) == 4);
    EXPECT_EQ(add(2, 3), 5);
}


class MyClassTest : public testing::Test {
protected:  
    void SetUp() // 初始化，在每个TEST_F中都会被调用
    {std::cout << "---SetUp---" << std::endl; }

    void TearDown() // 销毁，在每个TEST_F结束时都会调用
    {std::cout << "---TearDown---" << std::endl; }
    
	// 可以定义一些成员变量，在TEST_F中能访问
    int var_a;
};

TEST_F(MyClassTest, HasPropertyA) {
    var_a=3;
    ASSERT_TRUE(add(var_a, 2) == 5);
    var_a=6;
    EXPECT_EQ(add(6, 3), 9);
}
TEST_F(MyClassTest, HasPropertyB) {
    var_a=6;
    EXPECT_EQ(add(var_a, 3), 7)  << "FAILED: EXPECT: 7, but given 9\n";
}


int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv); // 初始化 GoogleTest，将命令行参数传递给gtest

    return RUN_ALL_TESTS(); // 执行所有的 test case
}
// g++ .\src\gtest_demo.cpp -o out\gtest_out.exe -I include\googletest -L include\googletest\lib -lgtest -std=c++17