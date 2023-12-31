#include <boost/thread.hpp>
#include <iostream>
#include <boost/version.hpp>
#include<boost/format.hpp>

void print_version()
{
    std::cout<<boost::format("---BOOST_VERSION:%1%\tBOOST_LIB_VERSION:%2%\n")%BOOST_VERSION%BOOST_LIB_VERSION;
}
void wait(int seconds)
{
    boost::this_thread::sleep(boost::posix_time::seconds(seconds));
}

void thread()
{
    try
    {
        for (int i = 0; i < 5; ++i)
        {
            wait(1);
            std::cout << i << std::endl;
        }
    }
    catch (boost::thread_interrupted &)
    {
    }
}

int main()
{
    print_version();
    boost::thread t(thread);
    wait(3);
    t.interrupt();
    t.join();
}

// g++ .\src\boost_demo.cpp -o out\boost_demo.exe -I include\boost_1_84_0 -L include\boost_1_84_0\lib -lboost_thread-mgw13-mt-x64-1_84 -std=c++11