/**
 * @file optimized_client.cpp
 * @brief 优化的客户端实现文件
 * @author cch
 * @date 2024-12-08
 * @version 2.0 - 使用类封装优化
*/

#include "optimized_client.h"
#include <sstream>
#include <iomanip>
#include <cstring>

using json = nlohmann::json;

// 构造函数
OptimizedClient::OptimizedClient(const std::string& ip, int port) 
    : serverIP(ip), serverPort(port), clientSocket(INVALID_SOCKET) {
    
    #ifdef _WIN32
    // Windows下初始化WSA
    WORD sockVersion = MAKEWORD(2, 2);
    WSADATA wsaData;
    if (WSAStartup(sockVersion, &wsaData) != 0) {
        logError("WSAStartup failed!");
        throw std::runtime_error("WSAStartup failed!");
    }
    #endif
    
    logMessage("OptimizedClient constructed successfully");
}

// 析构函数
OptimizedClient::~OptimizedClient() {
    disconnect();
    cleanup();
    logMessage("OptimizedClient destroyed");
}

// 连接到服务器
bool OptimizedClient::connect() {
    if (status.load() == ConnectionStatus::CONNECTED) {
        logMessage("Already connected to server");
        return true;
    }
    
    updateStatus(ConnectionStatus::CONNECTING);
    
    if (!createSocket()) {
        updateStatus(ConnectionStatus::CONNECTION_ERROR);
        return false;
    }
    
    if (!establishConnection()) {
        updateStatus(ConnectionStatus::CONNECTION_ERROR);
        return false;
    }
    
    updateStatus(ConnectionStatus::CONNECTED);
    running.store(true);
    
    // 启动接收线程
    receiveThread = std::thread(&OptimizedClient::receiveLoop, this);
    
    // 启动重连线程
    if (reconnectEnabled.load()) {
        reconnectThread = std::thread(&OptimizedClient::reconnectLoop, this);
    }
    
    logMessage("Connected to server " + serverIP + ":" + std::to_string(serverPort));
    return true;
}

// 断开连接
void OptimizedClient::disconnect() {
    if (status.load() == ConnectionStatus::DISCONNECTED) {
        return;
    }
    
    running.store(false);
    updateStatus(ConnectionStatus::DISCONNECTED);
    
    // 关闭socket
    if (clientSocket != INVALID_SOCKET) {
        #ifdef __linux__
        close(clientSocket);
        #elif _WIN32
        closesocket(clientSocket);
        #endif
        clientSocket = INVALID_SOCKET;
    }
    
    // 等待线程结束
    if (receiveThread.joinable()) {
        receiveThread.join();
    }
    
    if (reconnectThread.joinable()) {
        reconnectThread.join();
    }
    
    logMessage("Disconnected from server");
}

// 发送消息
bool OptimizedClient::sendMessage(const std::string& message) {
    if (status.load() != ConnectionStatus::CONNECTED) {
        logError("Not connected to server");
        return false;
    }
    
    int bytesSent = send(clientSocket, message.c_str(), message.length(), 0);
    if (bytesSent == SOCKET_ERROR) {
        logError("Failed to send message");
        updateStatus(ConnectionStatus::CONNECTION_ERROR);
        return false;
    }
    
    sentMessageCount++;
    lastActivity.store(std::chrono::steady_clock::now());
    logMessage("Sent message: " + message);
    return true;
}

// 发送JSON请求
bool OptimizedClient::sendJsonRequest(const std::string& function, int num1, int num2) {
    try {
        json request;
        request["function"] = function;
        request["params"]["num1"] = num1;
        request["params"]["num2"] = num2;
        
        std::string jsonStr = request.dump();
        return sendMessage(jsonStr);
        
    } catch (const std::exception& e) {
        logError("Failed to create JSON request: " + std::string(e.what()));
        return false;
    }
}

// 获取接收到的消息
std::vector<Message> OptimizedClient::getReceivedMessages() {
    std::lock_guard<std::mutex> lock(messagesMutex);
    return receivedMessages;
}

// 清空接收到的消息
void OptimizedClient::clearReceivedMessages() {
    std::lock_guard<std::mutex> lock(messagesMutex);
    receivedMessages.clear();
}

// 设置消息回调
void OptimizedClient::setMessageCallback(std::function<void(const std::string&)> callback) {
    std::lock_guard<std::mutex> lock(callbackMutex);
    messageCallback = callback;
}

// 设置状态回调
void OptimizedClient::setStatusCallback(std::function<void(ConnectionStatus)> callback) {
    std::lock_guard<std::mutex> lock(callbackMutex);
    statusCallback = callback;
}

// 创建Socket
bool OptimizedClient::createSocket() {
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        logError("Failed to create socket");
        return false;
    }
    
    // 设置端口重用
    int opt = 1;
    if (setsockopt(clientSocket, SOL_SOCKET, SO_REUSEADDR, 
                   (const char*)&opt, sizeof(opt)) == SOCKET_ERROR) {
        logError("Failed to set socket options");
        return false;
    }
    
    return true;
}

// 建立连接
bool OptimizedClient::establishConnection() {
    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    
    #ifdef __linux__
    serverAddr.sin_addr.s_addr = inet_addr(serverIP.c_str());
    #elif _WIN32
    serverAddr.sin_addr.S_un.S_addr = inet_addr(serverIP.c_str());
    #endif
    
    // connect 函数名与类的方法名冲突了。我需要使用作用域解析运算符来调用系统的 connect 函数。
    if (::connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        logError("Failed to connect to server");
        return false;
    }
    
    return true;
}

// 接收循环
void OptimizedClient::receiveLoop() {
    const int bufferSize = 1024;
    char buffer[bufferSize];
    
    logMessage("Receive loop started");
    
    while (running.load() && status.load() == ConnectionStatus::CONNECTED) {
        memset(buffer, 0, bufferSize);
        int bytesReceived = recv(clientSocket, buffer, bufferSize - 1, 0);
        
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            std::string message(buffer);
            
            addReceivedMessage(message);
            lastActivity.store(std::chrono::steady_clock::now());
            
            logMessage("Received: " + message);
            
            // 调用消息回调
            {
                std::lock_guard<std::mutex> lock(callbackMutex);
                if (messageCallback) {
                    messageCallback(message);
                }
            }
            
        } else if (bytesReceived == 0) {
            logMessage("Server disconnected");
            updateStatus(ConnectionStatus::DISCONNECTED);
            break;
        } else {
            logError("Receive error");
            updateStatus(ConnectionStatus::CONNECTION_ERROR);
            break;
        }
    }
    
    logMessage("Receive loop ended");
}

// 重连循环
void OptimizedClient::reconnectLoop() {
    logMessage("Reconnect loop started");
    
    while (running.load() && reconnectEnabled.load()) {
        if (status.load() == ConnectionStatus::DISCONNECTED || 
            status.load() == ConnectionStatus::CONNECTION_ERROR) {
            
            logMessage("Attempting to reconnect...");
            
            // 关闭旧连接
            if (clientSocket != INVALID_SOCKET) {
                #ifdef __linux__
                close(clientSocket);
                #elif _WIN32
                closesocket(clientSocket);
                #endif
                clientSocket = INVALID_SOCKET;
            }
            
            // 尝试重连
            if (createSocket() && establishConnection()) {
                updateStatus(ConnectionStatus::CONNECTED);
                logMessage("Reconnected successfully");
                
                // 重新启动接收线程
                if (receiveThread.joinable()) {
                    receiveThread.join();
                }
                receiveThread = std::thread(&OptimizedClient::receiveLoop, this);
            } else {
                logError("Reconnection failed");
                updateStatus(ConnectionStatus::CONNECTION_ERROR);
            }
        }
        
        // 等待重连间隔
        std::this_thread::sleep_for(std::chrono::seconds(reconnectInterval.load()));
    }
    
    logMessage("Reconnect loop ended");
}

// 更新状态
void OptimizedClient::updateStatus(ConnectionStatus newStatus) {
    ConnectionStatus oldStatus = status.load();
    status.store(newStatus);
    
    // 调用状态回调
    {
        std::lock_guard<std::mutex> lock(callbackMutex);
        if (statusCallback && oldStatus != newStatus) {
            statusCallback(newStatus);
        }
    }
}

// 添加接收到的消息
void OptimizedClient::addReceivedMessage(const std::string& message) {
    std::lock_guard<std::mutex> lock(messagesMutex);
    
    // 检查是否是JSON消息
    bool isJson = (message.find('{') != std::string::npos && message.find('}') != std::string::npos);
    
    receivedMessages.emplace_back(message, isJson);
    receivedMessageCount++;
    
    // 限制消息历史记录数量
    if (receivedMessages.size() > 1000) {
        receivedMessages.erase(receivedMessages.begin());
    }
}

// 日志方法
void OptimizedClient::logMessage(const std::string& message) const {
    std::cout << "[" << getCurrentTime() << "] [CLIENT-INFO] " << message << std::endl;
}

void OptimizedClient::logError(const std::string& error) const {
    std::cerr << "[" << getCurrentTime() << "] [CLIENT-ERROR] " << error << std::endl;
}

// 获取当前时间字符串
std::string OptimizedClient::getCurrentTime() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

// 清理资源
void OptimizedClient::cleanup() {
    #ifdef _WIN32
    WSACleanup();
    #endif
} 