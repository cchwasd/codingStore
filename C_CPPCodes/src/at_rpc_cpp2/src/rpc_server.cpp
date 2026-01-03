// rpc_server.cpp
#include "at_protocol.h"
#include "network_utils.h" // Includes socket setup/cleanup
#include <iostream>
#include <thread>
#include <sstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class RPCServer {
public:
    RPCServer(const std::string& host, int port)
        : host_(host), port_(port), server_socket_(INVALID_SOCKET), running_(false) {
        AT_Protocol_ = std::make_unique<ATProtocol>();
    }

    bool start() {
        initialize_sockets();

        server_socket_ = create_socket();
        if (server_socket_ == INVALID_SOCKET) {
            spdlog::critical("Failed to create server socket.");
            return false;
        }

        if (!bind_socket(server_socket_, host_, port_)) {
            spdlog::critical("Failed to bind server socket to {}:{}", host_, port_);
            close_socket(server_socket_);
            cleanup_sockets();
            return false;
        }

        if (::listen(server_socket_, SOMAXCONN) == SOCKET_ERROR) {
            int err = get_last_error();
            spdlog::critical("Listen failed: {}", get_error_message(err));
            close_socket(server_socket_);
            cleanup_sockets();
            return false;
        }

        spdlog::info("RPC Server listening on {}:{}", host_, port_);
        running_ = true;

        while (running_) {
            sockaddr_in client_addr{};
            socklen_t client_addr_len = sizeof(client_addr);
            socket_t client_socket = ::accept(server_socket_, (sockaddr*)&client_addr, &client_addr_len);
            if (client_socket == INVALID_SOCKET) {
                if(running_) { // Check if shutdown was intended
                     int err = get_last_error();
                     spdlog::error("Accept failed: {}", get_error_message(err));
                }
                continue; // Continue loop even on accept failure, unless shutting down
            }
            spdlog::info("Accepted connection");

            // Handle each client in a separate thread
            std::thread([this, client_socket]() {
                handle_client(client_socket);
                close_socket(client_socket);
                spdlog::info("Client disconnected");
            }).detach(); // Detach thread to allow independent execution
        }

        cleanup_sockets();
        return true;
    }

    void stop() {
        running_ = false;
        if (server_socket_ != INVALID_SOCKET) {
             // Trigger accept to wake up if blocked
             socket_t temp_sock = create_socket();
             if(temp_sock != INVALID_SOCKET) {
                 connect_socket(temp_sock, host_, port_); // Connect to self briefly
                 close_socket(temp_sock);
             }
             close_socket(server_socket_);
             server_socket_ = INVALID_SOCKET;
        }
        spdlog::info("RPC Server stopped.");
    }

    ~RPCServer() {
        if (running_) {
            stop();
        }
    }

private:
    std::string host_;
    int port_;
    socket_t server_socket_;
    std::atomic<bool> running_;
    std::unique_ptr<ATProtocol> AT_Protocol_;

    double perform_calculation(const std::string& func, const std::vector<double>& args) {
        if (func == "add") {
            if (args.size() != 2) throw std::invalid_argument("add requires 2 arguments");
            return args[0] + args[1];
        } else if (func == "subtract") {
            if (args.size() != 2) throw std::invalid_argument("subtract requires 2 arguments");
            return args[0] - args[1];
        } else if (func == "multiply") {
            if (args.size() != 2) throw std::invalid_argument("multiply requires 2 arguments");
            return args[0] * args[1];
        } else if (func == "divide") {
            if (args.size() != 2) throw std::invalid_argument("divide requires 2 arguments");
            if (args[1] == 0) throw std::invalid_argument("Division by zero");
            return args[0] / args[1];
        } else {
            throw std::invalid_argument("Unknown function: " + func);
        }
    }

    void handle_client(socket_t client_socket) {
    try {
        while (true) { // Keep handling requests on the same connection
            // 1. Receive Header
            std::vector<uint8_t> header_buffer(ATProtocol::HEADER_SIZE);
            if (recv_all(client_socket, header_buffer, ATProtocol::HEADER_SIZE) != ATProtocol::HEADER_SIZE) {
                spdlog::info("Client disconnected or error receiving header.");
                break; // Client disconnected or error
            }

            // 2. Parse Header to get body length
            ATHeader temp_header;
            std::memcpy(&temp_header, header_buffer.data(), ATProtocol::HEADER_SIZE);

            uint32_t body_len = temp_header.body_length;

            // 3. Receive Body
            std::vector<uint8_t> body_buffer(body_len);
            if (recv_all(client_socket, body_buffer, body_len) != body_len) {
                spdlog::error("Error receiving body data.");
                break;
            }

            // 4. Combine Header and Body for unpacking
            // 注意：AT_Protocol_->unpack 似乎可以直接处理分开的 header 和 body，
            // 或者需要组合后的 full_packet。请根据你的 unpack 实现调整。
            // 如果 unpack 需要完整包，则保留下面代码；否则可以直接用 header_buffer 和 body_buffer。
            std::vector<uint8_t> full_packet;
            full_packet.insert(full_packet.end(), header_buffer.begin(), header_buffer.end());
            full_packet.insert(full_packet.end(), body_buffer.begin(), body_buffer.end());

            // 5. Unpack Request
            uint16_t flags;
            uint32_t sequence;
            std::string request_body_str;
            // --- 关键修改 1: 增加 unpack 错误处理 ---
            try {
                 AT_Protocol_->unpack(full_packet, flags, sequence, request_body_str);
            } catch (const std::exception& e) {
                 spdlog::error("AT_Protocol_->unpack failed: {}. Disconnecting client.", e.what());
                 // 如果 unpack 失败，通常意味着协议错误，很难恢复，断开连接比较安全
                 break;
            }
            // --- 结束修改 1 ---

            if (!(flags & ATProtocol::FLAG_REQUEST)) {
                spdlog::warn("Received non-request packet (Seq: {}, Flags: 0x{:04X}). Ignoring.", sequence, flags);
                continue; // Expecting a request
            }

            json request_json;
            try {
                request_json = json::parse(request_body_str);
            } catch (const json::exception& e) {
                spdlog::error("JSON parse error in request (Seq: {}): {}", sequence, e.what());
                // Send error response
                json error_response = {{"status", "error"}, {"message", "Invalid JSON in request"}};
                std::string response_body = error_response.dump();
                auto response_packet = AT_Protocol_->pack(ATProtocol::FLAG_ERROR, response_body, sequence);
                spdlog::debug("Sending JSON parse error response ({} bytes)", response_packet.size()); // 添加日志
                if (send_all(client_socket, response_packet) != response_packet.size()) {
                     spdlog::error("Failed to send JSON parse error response packet (Seq: {}). Disconnecting.", sequence);
                     break; // Assume connection issue
                }
                continue;
            }

            std::string func_name = request_json.value("func", "");
            std::vector<double> args_vec;
            try {
                // 使用 is_array 检查更安全
                if (!request_json.contains("args") || !request_json.at("args").is_array()) {
                     throw json::type_error::create(302, "type must be array, but is " + std::string(request_json.at("args").type_name()), &request_json);
                }
                for (const auto& arg : request_json.at("args")) {
                    // 检查类型更安全
                    if (!arg.is_number()) {
                         throw json::type_error::create(302, "array element type must be number", &arg);
                    }
                    args_vec.push_back(arg.get<double>());
                }
            } catch (const json::exception& e) {
                spdlog::error("Error parsing arguments from JSON (Seq: {}): {}", sequence, e.what());
                json error_response = {{"status", "error"}, {"message", "Invalid arguments format"}};
                std::string response_body = error_response.dump();
                auto response_packet = AT_Protocol_->pack(ATProtocol::FLAG_ERROR, response_body, sequence);
                spdlog::debug("Sending argument parse error response ({} bytes)", response_packet.size()); // 添加日志
                if (send_all(client_socket, response_packet) != response_packet.size()) {
                     spdlog::error("Failed to send argument parse error response packet (Seq: {}). Disconnecting.", sequence);
                     break; // Assume connection issue
                }
                continue;
            }

            // 6. Perform Calculation and Prepare Response
            // --- 关键修改 2: 明确初始化 response_json ---
            json response_json = {{"status", "error"}, {"message", "Unknown error"}}; // 默认错误响应
            uint16_t response_flags = ATProtocol::FLAG_ERROR; // 默认错误标志
            try {
                double result = perform_calculation(func_name, args_vec);
                response_json = {{"status", "success"}, {"result", result}}; // 成功则覆盖
                response_flags = ATProtocol::FLAG_RESPONSE; // 成功标志
                spdlog::debug("Calculation successful for '{}', result: {}", func_name, result); // 添加成功日志
            } catch (const std::exception& e) {
                spdlog::error("Calculation error for '{}' (Seq: {}): {}", func_name, sequence, e.what());
                response_json = {{"status", "error"}, {"message", std::string(e.what())}}; // 使用异常信息
                // response_flags 保持 ATProtocol::FLAG_ERROR
            } catch (...) { // 捕获所有其他异常
                spdlog::critical("Unknown exception type caught during calculation for '{}' (Seq: {})!", func_name, sequence);
                response_json = {{"status", "error"}, {"message", "Critical internal server error"}};
                // response_flags 保持 ATProtocol::FLAG_ERROR
            }
            // --- 结束修改 2 ---

            // 7. Pack and Send Response
            std::string response_body = response_json.dump();
            // response_flags 已在上面根据情况设置
            auto response_packet = AT_Protocol_->pack(response_flags, response_body, sequence);

            // --- 关键修改 3: 增强 send_all 调用的日志 ---
            spdlog::debug("Preparing to send response (Seq: {}, Status: {}, Size: {} bytes): {}", sequence, response_json.value("status", "unknown"), response_packet.size(), response_json.dump());
            size_t send_success = send_all(client_socket, response_packet);
            if (send_success != response_packet.size()) {
                spdlog::error("Failed to send response packet (Seq: {}, Status: {}). Disconnecting.", sequence, response_json.value("status", "unknown"));
                break; // Assume connection issue
            } else {
                spdlog::info("Successfully processed and sent response for request '{}' with seq {} -> {}", func_name, sequence, response_json.dump());
            }
            // --- 结束修改 3 ---
        }
    } catch (const std::exception& e) {
        spdlog::error("Unhandled std::exception in handle_client: {}", e.what());
    } catch (...) {
         spdlog::critical("Unhandled unknown exception type in handle_client!");
    }
    // Close the client socket when done or on error
    closesocket(client_socket); // Don't forget to close the socket
    spdlog::info("Client connection closed.");
    }
};

// Global pointer for signal handler access (simplified approach)
RPCServer* g_server_instance = nullptr;

// Example signal handler (Unix/Linux) - Optional enhancement
/*
#ifdef __linux__
#include <signal.h>
void signal_handler(int signal) {
    spdlog::info("Received signal {}, stopping server...", signal);
    if(g_server_instance) {
        g_server_instance->stop();
    }
}
#endif
*/

void run_server(const std::string& host, int port) {
    RPCServer server(host, port);
    g_server_instance = &server; // Set global instance pointer

    /*
    #ifdef __linux__
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    #endif
    */

    server.start(); // This will block until stop() is called or error occurs
    g_server_instance = nullptr;
}