/**
 * @file optimized_demo.cpp
 * @brief 优化后的演示程序，展示如何使用新的类封装
 * @author cch
 * @date 2024-12-08
 * @version 2.0 - 使用类封装优化
*/

#include "optimized_server.h"
#include "optimized_client.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <memory>
#include <atomic>

// 全局变量用于控制程序退出
std::atomic<bool> g_running{true};

// 服务器演示函数
void demoServer() {
    std::cout << "\n=== 服务器演示 ===" << std::endl;
    
    try {
        // 创建服务器实例
        OptimizedServer server("127.0.0.1", 6006);
        
        // 注册自定义函数
        server.registerFunction("power", [](int a, int b) -> int {
            int result = 1;
            for (int i = 0; i < b; ++i) {
                result *= a;
            }
            return result;
        });
        
        server.registerFunction("max", [](int a, int b) -> int {
            return (a > b) ? a : b;
        });
        
        // 启动服务器
        if (!server.start()) {
            std::cerr << "服务器启动失败！" << std::endl;
            return;
        }
        
        std::cout << "服务器启动成功，等待客户端连接..." << std::endl;
        std::cout << "按 'q' 退出服务器" << std::endl;
        
        // 主循环
        while (g_running.load()) {
            std::string input;
            std::getline(std::cin, input);
            
            if (input == "q" || input == "quit") {
                break;
            } else if (input == "status") {
                std::cout << "连接客户端数量: " << server.getConnectedClients() << std::endl;
                auto clientList = server.getClientList();
                if (!clientList.empty()) {
                    std::cout << "客户端列表:" << std::endl;
                    for (const auto& client : clientList) {
                        std::cout << "  - " << client << std::endl;
                    }
                } else {
                    std::cout << "  无客户端连接" << std::endl;
                }
            } else if (input == "help") {
                std::cout << "可用命令:" << std::endl;
                std::cout << "  status - 显示连接状态" << std::endl;
                std::cout << "  q/quit - 退出服务器" << std::endl;
                std::cout << "  help - 显示帮助信息" << std::endl;
            }
        }
        
        // 停止服务器
        server.stop();
        std::cout << "服务器已停止" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "服务器异常: " << e.what() << std::endl;
    }
}

// 客户端演示函数
void demoClient() {
    std::cout << "\n=== 客户端演示 ===" << std::endl;
    
    try {
        // 创建客户端实例
        OptimizedClient client("127.0.0.1", 6006);
        
        // 设置消息回调
        client.setMessageCallback([](const std::string& message) {
            std::cout << "收到服务器响应: " << message << std::endl;
        });
        
        // 设置状态回调
        client.setStatusCallback([](ConnectionStatus status) {
            switch (status) {
                case ConnectionStatus::CONNECTED:
                    std::cout << "已连接到服务器" << std::endl;
                    break;
                case ConnectionStatus::DISCONNECTED:
                    std::cout << "与服务器断开连接" << std::endl;
                    break;
                case ConnectionStatus::CONNECTING:
                    std::cout << "正在连接服务器..." << std::endl;
                    break;
                case ConnectionStatus::CONNECTION_ERROR:
                    std::cout << "连接错误" << std::endl;
                    break;
            }
        });
        
        // 连接到服务器
        if (!client.connect()) {
            std::cerr << "连接服务器失败！" << std::endl;
            return;
        }
        
        std::cout << "客户端连接成功！" << std::endl;
        std::cout << "可用命令:" << std::endl;
        std::cout << "  add <num1> <num2> - 加法运算" << std::endl;
        std::cout << "  sub <num1> <num2> - 减法运算" << std::endl;
        std::cout << "  mul <num1> <num2> - 乘法运算" << std::endl;
        std::cout << "  div <num1> <num2> - 除法运算" << std::endl;
        std::cout << "  power <num1> <num2> - 幂运算" << std::endl;
        std::cout << "  max <num1> <num2> - 最大值" << std::endl;
        std::cout << "  hello <name> - 发送问候" << std::endl;
        std::cout << "  status - 显示状态" << std::endl;
        std::cout << "  q/quit - 退出客户端" << std::endl;
        
        // 主循环
        while (g_running.load() && client.isConnected()) {
            std::string input;
            std::getline(std::cin, input);
            
            if (input == "q" || input == "quit") {
                break;
            } else if (input == "status") {
                std::cout << "连接状态: " << (client.isConnected() ? "已连接" : "未连接") << std::endl;
                std::cout << "发送消息数: " << client.getSentMessageCount() << std::endl;
                std::cout << "接收消息数: " << client.getReceivedMessageCount() << std::endl;
            } else if (input.substr(0, 4) == "add ") {
                int num1, num2;
                if (sscanf(input.c_str(), "add %d %d", &num1, &num2) == 2) {
                    client.sendJsonRequest("add", num1, num2);
                } else {
                    std::cout << "格式错误，请使用: add <num1> <num2>" << std::endl;
                }
            } else if (input.substr(0, 4) == "sub ") {
                int num1, num2;
                if (sscanf(input.c_str(), "sub %d %d", &num1, &num2) == 2) {
                    client.sendJsonRequest("sub", num1, num2);
                } else {
                    std::cout << "格式错误，请使用: sub <num1> <num2>" << std::endl;
                }
            } else if (input.substr(0, 4) == "mul ") {
                int num1, num2;
                if (sscanf(input.c_str(), "mul %d %d", &num1, &num2) == 2) {
                    client.sendJsonRequest("mul", num1, num2);
                } else {
                    std::cout << "格式错误，请使用: mul <num1> <num2>" << std::endl;
                }
            } else if (input.substr(0, 4) == "div ") {
                int num1, num2;
                if (sscanf(input.c_str(), "div %d %d", &num1, &num2) == 2) {
                    client.sendJsonRequest("div", num1, num2);
                } else {
                    std::cout << "格式错误，请使用: div <num1> <num2>" << std::endl;
                }
            } else if (input.substr(0, 6) == "power ") {
                int num1, num2;
                if (sscanf(input.c_str(), "power %d %d", &num1, &num2) == 2) {
                    client.sendJsonRequest("power", num1, num2);
                } else {
                    std::cout << "格式错误，请使用: power <num1> <num2>" << std::endl;
                }
            } else if (input.substr(0, 4) == "max ") {
                int num1, num2;
                if (sscanf(input.c_str(), "max %d %d", &num1, &num2) == 2) {
                    client.sendJsonRequest("max", num1, num2);
                } else {
                    std::cout << "格式错误，请使用: max <num1> <num2>" << std::endl;
                }
            } else if (input.substr(0, 6) == "hello ") {
                std::string name = input.substr(6);
                client.sendMessage(name);
            } else if (!input.empty()) {
                client.sendMessage(input);
            }
        }
        
        // 断开连接
        client.disconnect();
        std::cout << "客户端已断开连接" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "客户端异常: " << e.what() << std::endl;
    }
}

// 多客户端并发测试
void demoMultipleClients() {
    std::cout << "\n=== 多客户端并发测试 ===" << std::endl;
    
    try {
        // 启动服务器
        OptimizedServer server("127.0.0.1", 6006);
        if (!server.start()) {
            std::cerr << "服务器启动失败！" << std::endl;
            return;
        }
        
        std::cout << "服务器启动成功，开始多客户端测试..." << std::endl;
        
        // 创建多个客户端
        std::vector<std::unique_ptr<OptimizedClient>> clients;
        std::vector<std::thread> clientThreads;
        
        const int clientCount = 5;
        
        for (int i = 0; i < clientCount; ++i) {
            auto client = std::make_unique<OptimizedClient>("127.0.0.1", 6006);
            
            // 设置消息回调
            client->setMessageCallback([i](const std::string& message) {
                std::cout << "客户端" << i << " 收到: " << message << std::endl;
            });
            
            clients.push_back(std::move(client));
        }
        
        // 连接所有客户端
        for (auto& client : clients) {
            if (!client->connect()) {
                std::cerr << "客户端连接失败！" << std::endl;
                return;
            }
        }
        
        std::cout << "所有客户端连接成功，开始发送测试消息..." << std::endl;
        
        // 为每个客户端创建线程发送消息
        for (int i = 0; i < clientCount; ++i) {
            clientThreads.emplace_back([&clients, i]() {
                auto& client = clients[i];
                
                // 发送不同类型的消息
                std::vector<std::pair<std::string, std::pair<int, int>>> tests = {
                    {"add", {10 + i, 20 + i}},
                    {"mul", {5 + i, 3 + i}},
                    {"power", {2, 3 + i}},
                    {"max", {100 + i, 50 + i}}
                };
                
                for (const auto& test : tests) {
                    client->sendJsonRequest(test.first, test.second.first, test.second.second);
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                }
                
                // 发送普通消息
                client->sendMessage("Client" + std::to_string(i) + " says hello!");
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            });
        }
        
        // 等待所有客户端线程完成
        for (auto& thread : clientThreads) {
            thread.join();
        }
        
        // 等待一段时间让服务器处理完所有消息
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        // 显示服务器状态
        std::cout << "\n服务器状态:" << std::endl;
        std::cout << "连接客户端数量: " << server.getConnectedClients() << std::endl;
        
        // 断开所有客户端
        for (auto& client : clients) {
            client->disconnect();
        }
        
        // 停止服务器
        server.stop();
        
        std::cout << "多客户端测试完成" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "多客户端测试异常: " << e.what() << std::endl;
    }
}

// 信号处理函数
void signalHandler(int signal) {
    (void)signal; // 避免未使用参数警告
    std::cout << "\n收到退出信号，正在关闭..." << std::endl;
    g_running.store(false);
}

int main() {
    std::cout << "=== 优化的Socket通讯演示程序 ===" << std::endl;
    std::cout << "版本: 2.0 - 使用类封装优化" << std::endl;
    std::cout << "作者: cch" << std::endl;
    std::cout << "日期: 2024-12-08" << std::endl;
    
    // 设置信号处理
    #ifdef __linux__
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    #endif
    
    while (g_running.load()) {
        std::cout << "\n请选择演示模式:" << std::endl;
        std::cout << "1. 服务器演示" << std::endl;
        std::cout << "2. 客户端演示" << std::endl;
        std::cout << "3. 多客户端并发测试" << std::endl;
        std::cout << "0. 退出程序" << std::endl;
        std::cout << "请输入选择 (0-3): ";
        
        int choice;
        std::cin >> choice;
        std::cin.ignore(); // 清除输入缓冲区
        
        switch (choice) {
            case 0:
                g_running.store(false);
                break;
            case 1:
                demoServer();
                break;
            case 2:
                demoClient();
                break;
            case 3:
                demoMultipleClients();
                break;
            default:
                std::cout << "无效选择，请重新输入" << std::endl;
                break;
        }
    }
    
    std::cout << "程序退出" << std::endl;
    return 0;
} 