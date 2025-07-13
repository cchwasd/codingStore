/**
 * @file optimized_server.cpp
 * @brief 优化的服务端实现文件
 * @author cch
 * @date 2024-12-08
 * @version 2.0 - 使用类封装优化
*/

#include "optimized_server.h"
#include <regex>
#include <sstream>
#include <iomanip>
#include <cstring>

using json = nlohmann::json;

// 默认函数实现
namespace {
    int add_func(int num1, int num2) { return num1 + num2; }
    int sub_func(int num1, int num2) { return num1 - num2; }
    int mul_func(int num1, int num2) { return num1 * num2; }
    int div_func(int num1, int num2) { 
        if (num2 == 0) throw std::runtime_error("Division by zero");
        return num1 / num2; 
    }
}

// 构造函数
OptimizedServer::OptimizedServer(const std::string& ip, int port) 
    : serverIP(ip), serverPort(port), listenSocket(INVALID_SOCKET) {
    
    #ifdef _WIN32
    // Windows下初始化WSA
    WORD sockVersion = MAKEWORD(2, 2);
    WSADATA wsaData;
    if (WSAStartup(sockVersion, &wsaData) != 0) {
        logError("WSAStartup failed!");
        throw std::runtime_error("WSAStartup failed!");
    }
    #endif
    
    initializeDefaultFunctions();
    logMessage("OptimizedServer constructed successfully");
}

// 析构函数
OptimizedServer::~OptimizedServer() {
    stop();
    cleanup();
    logMessage("OptimizedServer destroyed");
}

// 初始化服务器
bool OptimizedServer::initialize() {
    if (!createSocket()) {
        logError("Failed to create socket");
        return false;
    }
    
    if (!bindSocket()) {
        logError("Failed to bind socket");
        return false;
    }
    
    if (!startListening()) {
        logError("Failed to start listening");
        return false;
    }
    
    logMessage("Server initialized successfully");
    return true;
}

// 启动服务器
bool OptimizedServer::start() {
    if (running.load()) {
        logError("Server is already running");
        return false;
    }
    
    if (!initialize()) {
        return false;
    }
    
    running.store(true);
    acceptThread = std::thread(&OptimizedServer::acceptLoop, this);
    
    logMessage("Server started successfully on " + serverIP + ":" + std::to_string(serverPort));
    return true;
}

// 停止服务器
void OptimizedServer::stop() {
    if (!running.load()) return;
    
    running.store(false);
    
    // 关闭监听socket
    if (listenSocket != INVALID_SOCKET) {
        #ifdef __linux__
        close(listenSocket);
        #elif _WIN32
        closesocket(listenSocket);
        #endif
        listenSocket = INVALID_SOCKET;
    }
    
    // 等待接受线程结束
    if (acceptThread.joinable()) {
        acceptThread.join();
    }
    
    // 断开所有客户端连接
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        for (auto& client : clients) {
            if (client && client->isConnected.load()) {
                client->isConnected.store(false);
                #ifdef __linux__
                close(client->socket);
                #elif _WIN32
                closesocket(client->socket);
                #endif
            }
        }
        clients.clear();
    }
    
    logMessage("Server stopped");
}

// 创建Socket
bool OptimizedServer::createSocket() {
    listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == INVALID_SOCKET) {
        logError("Failed to create socket");
        return false;
    }
    
    // 设置端口重用
    int opt = 1;
    if (setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, 
                   (const char*)&opt, sizeof(opt)) == SOCKET_ERROR) {
        logError("Failed to set socket options");
        return false;
    }
    
    return true;
}

// 绑定Socket
bool OptimizedServer::bindSocket() {
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(serverPort);
    
    #ifdef __linux__
    addr.sin_addr.s_addr = INADDR_ANY;
    #elif _WIN32
    addr.sin_addr.S_un.S_addr = INADDR_ANY;
    #endif
    
    if (bind(listenSocket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        logError("Failed to bind socket");
        return false;
    }
    
    return true;
}

// 开始监听
bool OptimizedServer::startListening() {
    if (listen(listenSocket, 5) == SOCKET_ERROR) {
        logError("Failed to start listening");
        return false;
    }
    
    return true;
}

// 接受连接循环
void OptimizedServer::acceptLoop() {
    logMessage("Accept loop started");
    
    while (running.load()) {
        sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        
        SOCKET clientSocket = accept(listenSocket, (sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket == INVALID_SOCKET) {
            if (running.load()) {
                logError("Accept failed");
            }
            break;
        }
        
        // 创建客户端信息
        std::string clientIP = inet_ntoa(clientAddr.sin_addr);
        int clientPort = ntohs(clientAddr.sin_port);
        int clientId = ++clientCounter;
        
        auto clientInfo = std::make_unique<ClientInfo>(clientSocket, clientIP, clientPort, clientId);
        
        logMessage("New client connected: " + clientIP + ":" + std::to_string(clientPort) + 
                  " (ID: " + std::to_string(clientId) + ")");
        
        // 创建线程处理客户端
        std::thread clientThread(&OptimizedServer::handleClient, this, std::move(clientInfo));
        clientThread.detach();
    }
    
    logMessage("Accept loop ended");
}

// 处理客户端连接
void OptimizedServer::handleClient(std::unique_ptr<ClientInfo> client) {
    const int bufferSize = 1024;
    char buffer[bufferSize];
    
    // 添加到客户端列表
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        clients.push_back(std::move(client));
    }
    
    auto& clientRef = clients.back();
    
    while (clientRef->isConnected.load() && running.load()) {
        memset(buffer, 0, bufferSize);
        int bytesReceived = recv(clientRef->socket, buffer, bufferSize - 1, 0);
        
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            std::string message(buffer);
            
            logMessage("From client " + std::to_string(clientRef->clientId) + ": " + message);
            
            std::string response;
            
            // 检查是否是JSON请求
            if (message.find('{') != std::string::npos) {
                response = processJsonRequest(message);
            } else if (message == "exit") {
                response = "Good bye!";
                break;
            } else {
                response = "Hello " + message + "!";
            }
            
            // 发送响应
            send(clientRef->socket, response.c_str(), response.length(), 0);
            
        } else if (bytesReceived == 0) {
            logMessage("Client " + std::to_string(clientRef->clientId) + " disconnected");
            break;
        } else {
            logError("Receive error from client " + std::to_string(clientRef->clientId));
            break;
        }
    }
    
    // 移除客户端
    removeClient(clientRef->clientId);
}

// 移除客户端
void OptimizedServer::removeClient(int clientId) {
    std::lock_guard<std::mutex> lock(clientsMutex);
    clients.erase(
        std::remove_if(clients.begin(), clients.end(),
            [clientId](const std::unique_ptr<ClientInfo>& client) {
                return client && client->clientId == clientId;
            }),
        clients.end()
    );
}

// 处理JSON请求
std::string OptimizedServer::processJsonRequest(const std::string& message) {
    try {
        // 提取JSON字符串
        std::regex jsonPattern(R"(\{.*\})");
        std::smatch match;
        
        if (!std::regex_search(message, match, jsonPattern)) {
            return createJsonResponse(FunctionResult(false, "No JSON string found"));
        }
        
        std::string jsonStr = match[0];
        json j = json::parse(jsonStr);
        
        std::string funcName = j["function"].get<std::string>();
        int num1 = j["params"]["num1"].get<int>();
        int num2 = j["params"]["num2"].get<int>();
        
        FunctionResult result = executeFunction(funcName, num1, num2);
        return createJsonResponse(result);
        
    } catch (const std::exception& e) {
        logError("JSON processing error: " + std::string(e.what()));
        return createJsonResponse(FunctionResult(false, "JSON processing error: " + std::string(e.what())));
    }
}

// 执行函数
FunctionResult OptimizedServer::executeFunction(const std::string& funcName, int num1, int num2) {
    std::lock_guard<std::mutex> lock(functionMutex);
    
    auto it = functionMap.find(funcName);
    if (it == functionMap.end()) {
        return FunctionResult(false, "Function not found: " + funcName);
    }
    
    try {
        int result = it->second(num1, num2);
        return FunctionResult(true, "Success", result);
    } catch (const std::exception& e) {
        return FunctionResult(false, "Function execution error: " + std::string(e.what()));
    }
}

// 创建JSON响应
std::string OptimizedServer::createJsonResponse(const FunctionResult& result) {
    json response;
    response["success"] = result.success;
    response["message"] = result.message;
    if (result.success) {
        response["result"] = result.result;
    }
    return response.dump();
}

// 获取连接客户端数量
int OptimizedServer::getConnectedClients() const {
    std::lock_guard<std::mutex> lock(clientsMutex);
    return static_cast<int>(clients.size());
}

// 获取客户端列表
std::vector<std::string> OptimizedServer::getClientList() const {
    std::lock_guard<std::mutex> lock(clientsMutex);
    std::vector<std::string> clientList;
    
    for (const auto& client : clients) {
        if (client && client->isConnected.load()) {
            clientList.push_back(client->ip + ":" + std::to_string(client->port) + 
                               " (ID: " + std::to_string(client->clientId) + ")");
        }
    }
    
    return clientList;
}

// 初始化默认函数
void OptimizedServer::initializeDefaultFunctions() {
    functionMap["add"] = add_func;
    functionMap["sub"] = sub_func;
    functionMap["mul"] = mul_func;
    functionMap["div"] = div_func;
    
    logMessage("Default functions initialized");
}

// 清理资源
void OptimizedServer::cleanup() {
    #ifdef _WIN32
    WSACleanup();
    #endif
}

// 日志方法
void OptimizedServer::logMessage(const std::string& message) const {
    std::cout << "[" << getCurrentTime() << "] [INFO] " << message << std::endl;
}

void OptimizedServer::logError(const std::string& error) const {
    std::cerr << "[" << getCurrentTime() << "] [ERROR] " << error << std::endl;
}

// 获取当前时间字符串
std::string OptimizedServer::getCurrentTime() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
} 