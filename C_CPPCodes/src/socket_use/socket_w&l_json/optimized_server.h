/**
 * @file optimized_server.h
 * @brief 优化的服务端头文件，基于TCP+JSON实现C++与Python间函数调用
 * @author cch
 * @date 2024-12-08
 * @version 2.0 - 使用类封装优化
*/

#pragma once

#include <iostream>
#include <string>
#include <map>
#include <functional>
#include <thread>
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>
#include <chrono>

// 系统宏判断 头文件处理 linux / windows
#ifdef __linux__
    #include <arpa/inet.h>
    #include <unistd.h>
    #define SOCKET int
    #define INVALID_SOCKET (SOCKET)(~0)
    #define SOCKET_ERROR (-1)
#elif _WIN32
    #include <winsock2.h>
    #include <windows.h>
    typedef int socklen_t;
#endif

// 包含完整的json头文件
#include <nlohmann/json.hpp>

// 客户端连接信息结构
struct ClientInfo {
    SOCKET socket;
    std::string ip;
    int port;
    int clientId;
    std::chrono::steady_clock::time_point connectTime;
    std::atomic<bool> isConnected{true};
    
    ClientInfo(SOCKET s, const std::string& ip_addr, int p, int id) 
        : socket(s), ip(ip_addr), port(p), clientId(id), 
          connectTime(std::chrono::steady_clock::now()) {}
};

// 函数执行结果结构
struct FunctionResult {
    bool success;
    std::string message;
    int result;
    
    FunctionResult(bool s = false, const std::string& msg = "", int r = 0) 
        : success(s), message(msg), result(r) {}
};

// 优化的服务器类
class OptimizedServer {
public:
    OptimizedServer(const std::string& ip = "127.0.0.1", int port = 6006);
    ~OptimizedServer();
    
    // 禁用拷贝构造和赋值
    OptimizedServer(const OptimizedServer&) = delete;
    OptimizedServer& operator=(const OptimizedServer&) = delete;
    
    // 主要接口
    bool initialize();
    bool start();
    void stop();
    bool isRunning() const { return running.load(); }
    
    // 统计信息
    int getConnectedClients() const;
    std::vector<std::string> getClientList() const;
    
    // 注册自定义函数
    template<typename Func>
    void registerFunction(const std::string& name, Func func) {
        std::lock_guard<std::mutex> lock(functionMutex);
        functionMap[name] = func;
    }

private:
    // 成员变量
    std::string serverIP;
    int serverPort;
    SOCKET listenSocket;
    std::atomic<bool> running{false};
    std::atomic<int> clientCounter{0};
    
    // 线程管理
    std::thread acceptThread;
    std::vector<std::unique_ptr<ClientInfo>> clients;
    mutable std::mutex clientsMutex;
    
    // 函数映射
    std::map<std::string, std::function<int(int, int)>> functionMap;
    mutable std::mutex functionMutex;
    
    // 内部方法
    bool createSocket();
    bool bindSocket();
    bool startListening();
    void acceptLoop();
    void handleClient(std::unique_ptr<ClientInfo> client);
    void removeClient(int clientId);
    
    // JSON处理
    std::string processJsonRequest(const std::string& jsonStr);
    FunctionResult executeFunction(const std::string& funcName, int num1, int num2);
    std::string createJsonResponse(const FunctionResult& result);
    
    // 工具方法
    void logMessage(const std::string& message) const;
    void logError(const std::string& error) const;
    std::string getCurrentTime() const;
    
    // 初始化默认函数
    void initializeDefaultFunctions();
    
    // 清理资源
    void cleanup();
}; 