/**
 * @file server_demo.cpp
 * @brief 服务端示例文件，基于 tcp+json 实现c++,python间函数调用
 * @author cch
 * @date 2024-12-08
*/

#include <iostream>
#include <cstdio>
#include <cstdlib> // std::exit
#include <cstring> // memset sprintf strlen
#include <chrono>
#include <thread>
#include <map>
#include <functional>
#include <regex>
#include <string>

// 系统宏判断 头文件处理  linux / windows
#ifdef __linux__
    #include <arpa/inet.h> // inet_ntop, htons, ntohs, INADDR_ANY, INET_ADDRSTRLEN
    #include <unistd.h>    // close
        // #include <sys/socket.h> // sockaddr_in,  socket(),  bind(), listen(), accept(), send(), recv()，SOCK_STREAM，AF_INET
        /*
        <arpa/inet.h>包含了<netinet/in.h>，而<netinet/in.h>包含了 <sys/socket.h>。
        所以实际使用时，只需要#include <arpa/inet.h>，不需要#include <sys/socket.h>
        */
    #define SOCKET int     // win10 SOCKET为unsigned __int64
    #define INVALID_SOCKET (SOCKET)(~0)
    #define SOCKET_ERROR (-1)
#elif _WIN32
    #include <winsock2.h> // -lwsock32
    #include <windows.h>
    // #pragma comment(lib, "ws2_32.lib") //VS
    typedef int socklen_t;  // windows 没有
#endif

#include <nlohmann/json.hpp>

#define SOCK_PORT 6006      // 宏定义一个端口号
#define SOCK_IP "127.0.0.1" // 宏定义IP地址


using json = nlohmann::json;

int add_func(int num1, int num2)
{
    return num1 + num2;
}
int sub_func(int num1, int num2)
{
    return num1 - num2;
}
int mul_func(int num1, int num2)
{
    return num1 * num2;
}
int div_func(int num1, int num2)
{
    if (num2 == 0)
    {
        throw std::runtime_error("Division by zero.");
    }
    return num1 / num2;
}

// 映射函数名到函数,
std::map<std::string, std::function<int(int, int)>> function_map = {
    {"add", add_func}, {"sub", sub_func}, {"mul", mul_func}, {"div", div_func}};

// 根据JSON执行函数，实际业务中应，针对不同参数 一个功能函数对应一个json处理调用函数
std::string executeFunction(std::string json_str)
{
    json j = json::parse(json_str); //  j = string(json_str);
    auto func_name = j["function"].get<std::string>();
    auto params = j["params"].get<json>();
    auto num1 = j["params"]["num1"].get<int>();
    auto num2 = j["params"]["num2"].get<int>();

    auto it = function_map.find(func_name);
    if (it != function_map.end())
    {

        int result = it->second(num1, num2);
        j["result"] = result;
    }
    else
    {
        std::cerr << "Function not found: " << func_name << std::endl;
        j["result"] = "Function not found!";
    }
    std::string res_str = j.dump();
    j.clear();
    return res_str;
}

int server_tcp_link(int cfd, sockaddr_in cddr)
{
    // 5. 和客户端通信
    int maxSize = 1024;
    // 接收数据
    char buf[maxSize]={0};           // 接收缓冲区

    while (true)
    {
        int len = recv(cfd, buf, sizeof(buf) - 1, 0); // 从客户端读取数据
        if (len > 0)
        {
            buf[len] = '\0';                                 // 添加结束符号
            std::cout << "From client：" << buf << std::endl; // 打印客户端发送的消息
            std::string res_str = "";
            if (strchr(buf, '{') != NULL)
            {
                std::string buf_str(buf);
                std::regex json_pattern(R"(\{.*\})");
                std::smatch match; // 用于存储匹配结果
                // 进行匹配
                if (std::regex_search(buf_str, match, json_pattern))
                {
                    // 匹配成功，match[0]包含整个匹配的字符串
                    std::string json_str = match[0];
                    res_str = executeFunction(json_str);
                    // std::cout << "Matched JSON string: " << json_str << std::endl;
                }
                else
                {
                    std::cout << "No JSON string matched." << std::endl;
                    res_str = "No JSON string matched.";
                }

                std::cout << "---json_str:" << res_str << std::endl;

                strcpy(buf, res_str.c_str());
                send(cfd, buf, strlen(buf) + sizeof(char), 0);
            }
            else if (strcmp(buf, "exit") == 0)
            {
                strcpy(buf, "Good bye!");
                send(cfd, buf, strlen(buf) + sizeof(char), 0);
                break;
            }
            else
            {
                int tmp_len = strlen(buf);
                char name_str[tmp_len] = {0};
                strcpy(name_str, buf);
                sprintf(buf, "Hello %s!", name_str); // 格式化字符串
                send(cfd, buf, strlen(buf) + sizeof(char), 0);
            }
        }
        else if (len == 0)
        {
            std::cout << "The client is disconnected..." << std::endl; // 客户端断开连接
            break;
        }
        else
        {
            std::cerr <<"recv error" << std::endl; // 错误处理
            break;
        }
    }
    printf("A closed connection from client：%s:%d \n", inet_ntoa(cddr.sin_addr), ntohs(cddr.sin_port));
    #ifdef __linux__
        close(cfd); // 关闭与客户端的连接
    #elif _WIN32
        closesocket(cfd);
    #endif
    return 0;
}

int test_server_tcp_link()
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
    // 1. 创建监听的套接字
    SOCKET lfd = socket(AF_INET, SOCK_STREAM, 0); // 创建一个TCP套接字
    if (lfd == INVALID_SOCKET)
    {
        std::cerr << "socket failed!" << std::endl; // 错误处理
        std::exit(EXIT_FAILURE);
    }

	//将端口号快速重用
    int opt = 1;
    if (setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt))==-1)
    {
        std::cerr << "setsockopt failed!" << std::endl;
        exit(EXIT_FAILURE);
    }

    // 2. 将socket()返回值和本地的IP端口绑定到一起
    sockaddr_in addr;                 // 用于存储地址信息
    addr.sin_family = AF_INET;        // 地址族，IPv4
    addr.sin_port = htons(SOCK_PORT); // 大端端口转换

    #ifdef __linux__
        addr.sin_addr.s_addr = INADDR_ANY; // INADDR_ANY: 绑定到任意IP地址 ;//inet_addr(SOCK_IP);
        // if (inet_pton(AF_INET, SOCK_IP, &addr.sin_addr.s_addr) <= 0) // 将IP地址转换为网络字节顺序
        // {
        //     perror("inet_pton"); // 错误处理
        //     std::exit(EXIT_FAILURE);
        // }

    #elif _WIN32
        addr.sin_addr.S_un.S_addr = INADDR_ANY; // inet_addr(SOCK_IP);
    #endif

    int ret = bind(lfd, (sockaddr *)&addr, sizeof(addr)); // 绑定套接字到地址
    if (ret == SOCKET_ERROR)
    {
        std::cerr << "bind failed!" << std::endl;  // 错误处理
        std::exit(EXIT_FAILURE);
    }

    // 3. 设置监听
    ret = listen(lfd, 5); // 开始监听
    if (ret == SOCKET_ERROR)
    {
        std::cerr << "listen failed!" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // 4. 阻塞等待并接受客户端连接
    sockaddr_in cliaddr;          // 用于存储客户端地址信息
    int clilen = sizeof(cliaddr); // 客户端地址结构的大小
    SOCKET cfd;
    std::cout << "Wait for connection..." << std::endl;
    while (true)
    {
        cfd = accept(lfd, (sockaddr *)&cliaddr, (socklen_t *)&clilen); // 接受客户端连接
        if (cfd == INVALID_SOCKET)
        {
            // perror("accept"); // 错误处理
            std::cerr << "accept failed!" << std::endl;
            std::exit(EXIT_FAILURE);
        }

        printf("Accept new connection from client：%s:%d \n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));

        std::thread client_thread(server_tcp_link, cfd, cliaddr); // 创建线程并处理客户端连接
        client_thread.detach();                          // 分离线程
    }
    #ifdef __linux__
        close(lfd); // 关闭监听套接字
    #elif _WIN32
        closesocket(lfd);
        WSACleanup();
    #endif

    return 0;
}

int main(int argc, char **argv)
{
    test_server_tcp_link();
    return 0;
}

// g++ server_demo.cpp -o server_demo -I ../include -lpthread
// g++ server_demo.cpp -o server_demo.exe -lwsock32  -I ../../include
