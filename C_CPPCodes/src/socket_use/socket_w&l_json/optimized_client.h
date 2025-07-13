/**
 * @file optimized_client.h
 * @brief 优化的客户端头文件，基于TCP+JSON实现C++与Python间函数调用
 * @author cch
 * @date 2024-12-08
 * @version 2.0 - 使用类封装优化
*/

#pragma once

#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>
#include <functional>
#include <vector>
#include <queue>
#include <nlohmann/json.hpp>

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

// 消息结构
struct Message {
    std::string content;
    std::chrono::steady_clock::time_point timestamp;
    bool isJson;
    
    Message(const std::string& msg, bool json = false) 
        : content(msg), timestamp(std::chrono::steady_clock::now()), isJson(json) {}
};

// 连接状态枚举
enum class ConnectionStatus {
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    CONNECTION_ERROR
};

// 优化的客户端类
class OptimizedClient {
public:
    OptimizedClient(const std::string& serverIP = "127.0.0.1", int serverPort = 6006);
    ~OptimizedClient();
    
    // 禁用拷贝构造和赋值
    OptimizedClient(const OptimizedClient&) = delete;
    OptimizedClient& operator=(const OptimizedClient&) = delete;
    
    // 主要接口
    bool connect();
    void disconnect();
    bool isConnected() const { return status.load() == ConnectionStatus::CONNECTED; }
    ConnectionStatus getStatus() const { return status.load(); }
    
    // 消息发送
    bool sendMessage(const std::string& message);
    bool sendJsonRequest(const std::string& function, int num1, int num2);
    
    // 消息接收
    std::vector<Message> getReceivedMessages();
    void clearReceivedMessages();
    
    // 回调函数设置
    void setMessageCallback(std::function<void(const std::string&)> callback);
    void setStatusCallback(std::function<void(ConnectionStatus)> callback);
    
    // 统计信息
    int getSentMessageCount() const { return sentMessageCount.load(); }
    int getReceivedMessageCount() const { return receivedMessageCount.load(); }
    std::chrono::steady_clock::time_point getLastActivity() const { return lastActivity.load(); }
    
    // 配置选项
    void setReconnectEnabled(bool enabled) { reconnectEnabled.store(enabled); }
    void setReconnectInterval(int seconds) { reconnectInterval.store(seconds); }
    void setConnectionTimeout(int seconds) { connectionTimeout.store(seconds); }

private:
    // 成员变量
    std::string serverIP;
    int serverPort;
    SOCKET clientSocket;
    std::atomic<ConnectionStatus> status{ConnectionStatus::DISCONNECTED};
    std::atomic<bool> running{false};
    
    // 线程管理
    std::thread receiveThread;
    std::thread reconnectThread;
    
    // 消息管理
    std::vector<Message> receivedMessages;
    mutable std::mutex messagesMutex;
    std::atomic<int> sentMessageCount{0};
    std::atomic<int> receivedMessageCount{0};
    std::atomic<std::chrono::steady_clock::time_point> lastActivity{std::chrono::steady_clock::now()};
    
    // 回调函数
    std::function<void(const std::string&)> messageCallback;
    std::function<void(ConnectionStatus)> statusCallback;
    mutable std::mutex callbackMutex;
    
    // 重连配置
    std::atomic<bool> reconnectEnabled{true};
    std::atomic<int> reconnectInterval{5};
    std::atomic<int> connectionTimeout{10};
    
    // 内部方法
    bool createSocket();
    bool establishConnection();
    void receiveLoop();
    void reconnectLoop();
    void updateStatus(ConnectionStatus newStatus);
    void addReceivedMessage(const std::string& message);
    
    // 工具方法
    void logMessage(const std::string& message) const;
    void logError(const std::string& error) const;
    std::string getCurrentTime() const;
    
    // 清理资源
    void cleanup();
}; 