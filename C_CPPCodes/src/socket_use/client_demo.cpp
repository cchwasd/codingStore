/**
 * @file client_demo.cpp
 * @brief 客户端示例文件，基于 tcp+json 实现c++与python间函数调用
 * @author cch
 * @date 2024-12-08
*/

#include <iostream> // std::cout, std::cerr
#include <cstdlib>  // std::exit
#include <cstring>  // memset, strlen
#include <string>
#include <chrono>
#include <thread>
#include <vector>

// 系统宏判断 头文件处理  linux / windows
#ifdef __linux__
    #include <arpa/inet.h> // socket, connect, inet_pton, htons
    #include <unistd.h>    // close, sleep

    #define SOCKET int // win10 SOCKET为unsigned __int64
    #define INVALID_SOCKET (SOCKET)(~0)
    #define SOCKET_ERROR (-1)
#elif _WIN32
    #include <winsock2.h> // -lwsock32
    #include <windows.h>
#endif

#define SOCK_PORT 6006      // 宏定义一个端口号
#define SOCK_IP "127.0.0.1" // 宏定义IP地址


int cliect_tcp()
{
    #ifdef __linux__

    #elif _WIN32
        // 初始化WSA
        WORD sockVersion = MAKEWORD(2, 2);
        WSADATA wsaData;
        if (WSAStartup(sockVersion, &wsaData) != 0)
        {
            std::cerr << "WSAStartup failed! " << std::endl;
            std::exit(EXIT_FAILURE);
        }
    #endif

    // 1. 创建通信的套接字
    SOCKET fd = socket(AF_INET, SOCK_STREAM, 0); // 创建一个TCP套接字
    if (fd == SOCKET_ERROR)
    {
        // perror("socket"); // 错误处理
        std::cerr << "socket failed!" << std::endl;
        std::exit(EXIT_FAILURE);
    }

	//将端口号快速重用
    int opt = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt))==-1)
    {
        std::cerr << "setsockopt failed!" << std::endl;
        exit(EXIT_FAILURE);
    }
    // 2. 连接服务器
    sockaddr_in addr;                 // 用于存储服务器地址信息
    addr.sin_family = AF_INET;        // 地址族，IPv4
    addr.sin_port = htons(SOCK_PORT); // 大端端口转换

    #ifdef __linux__
        addr.sin_addr.s_addr = inet_addr(SOCK_IP);
        // if (inet_pton(AF_INET, SOCK_IP, &addr.sin_addr.s_addr) <= 0) // 将IP地址转换为网络字节顺序
        // {
        //     perror("inet_pton"); // 错误处理
        //     std::exit(EXIT_FAILURE);
        // }

    #elif _WIN32
        addr.sin_addr.S_un.S_addr = inet_addr(SOCK_IP);
    #endif

    int ret = connect(fd, (sockaddr *)&addr, sizeof(addr)); // 连接到服务器
    if (ret == SOCKET_ERROR)
    {
        std::cerr << "connect failed!" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // 3. 和服务器端通信
    int buf_size = 1024;
    char buf[buf_size];          // 数据缓冲区
    memset(buf, 0, sizeof(buf)); // 清空缓冲区
    std::vector<std::string> vstr = {"Michael", "Tracy", "Sarah", 
    R"(xxxx{"function": "add", "params": {"num1": 10, "num2": 20}}xxxx)", 
    R"(xxxx{"function": "mul", "params": {"num1": 10, "num2": 20}}xxxx)",
    R"(xxxx{"function": "xxx", "params": {"num1": 10, "num2": 20}}xxxx)",
    "exit"};
    for (int i = 0; i < vstr.size(); i++)
    {
        // 发送数据
        strcpy(buf, vstr.at(i).c_str());
        send(fd, buf, strlen(buf) + sizeof(char), 0); // 发送数据
        // 接收数据
        memset(buf, 0, sizeof(buf));                 // 清空缓冲区
        int len = recv(fd, buf, sizeof(buf) - 1, 0); // 从服务器读取数据
        if (len > 0)
        {
            buf[len] = '\0';
            std::cout << "From server: " << buf << std::endl;
            ; // 打印服务器发送的消息
        }
        else if (len == 0)
        {
            std::cout << "The server is disconnected..." << std::endl; // 服务器断开连接
            break;
        }
        else
        {
            std::cerr << "recv failed!" << std::endl;
            break;
        }
        // sleep(1); // 每隔1秒发送一条数据
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    #ifdef __linux__
        close(fd); // 关闭监听套接字
    #elif _WIN32
        closesocket(fd);
        WSACleanup();
    #endif

    return 0;
}

int main(int argc, char **argv)
{
    // cliect_tcp();
    for(int i=0; i<10; i++){
        std::thread client_thread(cliect_tcp); // 创建线程并模拟多个客户端请求
        client_thread.detach();
        if (i%2==0)
            std::this_thread::sleep_for(std::chrono::seconds(1));
        else
            std::this_thread::sleep_for(std::chrono::seconds(3));
    }

    return 0;
}
// g++ client_demo.cpp -o client_demo  -lpthread
// g++ client_demo.cpp -o client_demo.exe -lwsock32