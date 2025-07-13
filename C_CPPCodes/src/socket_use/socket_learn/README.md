# 多线程Socket通讯项目

## 项目概述

本项目演示了如何使用多线程技术实现Socket的一对多通讯，包括服务器端和客户端的完整实现。

## 技术特点

- **多线程并发处理**：服务器为每个客户端连接创建独立线程
- **异步通讯**：客户端使用独立线程进行数据接收
- **线程安全**：使用互斥锁保护共享资源
- **Windows Socket API**：基于WinSock2实现

## 项目结构

```
socket_learn/
├── mulThreadServer.h      # 服务器头文件
├── mulThreadServer.cpp    # 服务器实现
├── mulThreadClient.h      # 客户端头文件
├── mulThreadClient.cpp    # 客户端实现
├── main.cpp              # 主程序（测试代码）
├── CMakeLists.txt        # CMake构建文件
└── README.md             # 项目说明
```

## 核心类说明

### SocketServerTest（服务器类）

**主要功能：**
- 创建和管理服务器Socket
- 接受客户端连接
- 为每个客户端创建独立线程
- 处理客户端消息

**关键方法：**
- `CreateSocket()`: 创建服务器Socket
- `BandSocket()`: 绑定地址和端口
- `ListenSocket()`: 开始监听
- `AcceptSocketManager()`: 接受客户端连接
- `ThreadClientRecv()`: 客户端消息处理线程

### MultipartiteClientSocketTest（客户端类）

**主要功能：**
- 创建客户端Socket
- 连接服务器
- 异步发送和接收消息

**关键方法：**
- `CreateSocket()`: 创建客户端Socket
- `Myconnect()`: 连接服务器
- `Mysend()`: 发送消息
- `Myrecv()`: 接收消息线程

## 编译和运行

### 使用CMake编译

```bash
# 创建构建目录
mkdir build
cd build

# 生成构建文件
cmake ..

# 编译项目
cmake --build .

# 运行程序
./bin/socket_test
```

### 使用Visual Studio

1. 打开CMakeLists.txt文件
2. 配置项目
3. 编译并运行

## 测试模式

程序提供三种测试模式：

1. **服务器测试**：启动服务器，等待客户端连接
2. **客户端测试**：启动单个客户端，连接服务器并发送消息
3. **多客户端测试**：自动启动服务器和多个客户端进行测试

## 使用示例

### 启动服务器
```bash
./socket_test
# 选择 1 - 服务器测试
```

### 启动客户端
```bash
./socket_test
# 选择 2 - 客户端测试
# 输入消息发送给服务器
```

### 多客户端测试
```bash
./socket_test
# 选择 3 - 多客户端测试
# 自动测试多个客户端并发连接
```

## 技术实现细节

### 服务器端多线程处理流程

1. **主线程**：负责接受新的客户端连接
2. **工作线程**：每个客户端连接对应一个独立线程
3. **线程管理**：使用vector存储线程信息，支持动态管理

### 客户端异步通讯

1. **主线程**：处理用户输入和消息发送
2. **接收线程**：独立线程处理服务器消息接收
3. **线程同步**：使用标志位控制线程生命周期

### 数据结构

```cpp
// 服务器线程结构
typedef struct serverThread {
    std::thread *t1 = nullptr;    // 线程指针
    bool isRuning = false;        // 运行标志
    int threadID = -1;           // 线程ID
    SOCKET csocket = -1;         // 客户端Socket
} Sthread;

// 客户端线程结构
typedef struct thread1 {
    std::thread *t1 = nullptr;    // 线程指针
    bool isRuning = false;        // 运行标志
} Mythread;
```

## 注意事项

1. **端口占用**：确保8888端口未被占用
2. **防火墙设置**：可能需要配置防火墙允许程序网络访问
3. **线程安全**：多线程环境下注意资源竞争
4. **错误处理**：程序包含基本的错误处理机制

## 扩展功能

可以基于此项目扩展以下功能：

- 消息协议设计
- 数据加密传输
- 心跳检测机制
- 连接池管理
- 负载均衡
- 日志记录系统

## 故障排除

### 常见问题

1. **编译错误**：确保安装了CMake和C++编译器
2. **连接失败**：检查服务器是否启动，端口是否被占用
3. **程序崩溃**：检查Socket资源是否正确释放

### 调试技巧

1. 使用调试器设置断点
2. 查看控制台输出信息
3. 检查网络连接状态
4. 验证线程创建和销毁 