// at_rpc_demo.cpp
#include "at_protocol.h"
#include <cxxopts.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>


// 计算器函数
json calculator_handler(const json& request) {
    static const std::map<std::string, std::function<double(double, double)>> functions = {
        {"add", [](double a, double b){ return a + b; }},
        {"sub", [](double a, double b){ return a - b; }},
        {"mul", [](double a, double b){ return a * b; }},
        {"div", [](double a, double b){
            if (b == 0) throw std::runtime_error("Division by zero");
            return a / b;
        }}
    };

    json response = {{"status", "error"}, {"message", "Function not found"}};
    try {
        std::string func_name = request["func"];
        if (functions.count(func_name)) {
            double a = request["args"][0];
            double b = request["args"][1];
            double result = functions.at(func_name)(a, b);
            response = {{"status", "success"}, {"result", result}};
        }
    } catch (const std::exception& e) {
        spdlog::error("Error executing function: {}", e.what());
        response = {{"status", "error"}, {"message", e.what()}};
    }
    return response;
}

void run_client(const std::string& host, int port, const std::string& func, const std::vector<double>& args, int timeout_seconds = 5) {
    initialize_sockets();

    socket_t sock = create_socket();
    if (sock == INVALID_SOCKET) {
        spdlog::critical("Failed to create client socket.");
        cleanup_sockets();
        throw std::runtime_error("Socket creation failed");
    }

    if (!connect_socket(sock, host, port)) {
        spdlog::critical("Failed to connect to {}:{}.", host, port);
        close_socket(sock);
        cleanup_sockets();
        throw std::runtime_error("Connection failed");
    }

    spdlog::info("Connected to {}:{}", host, port);

    try {
        // Format request
        json request_json = {{"func", func}, {"args", args}};
        std::string request_body = request_json.dump();

        // Pack request
        ATProtocol protocol;
        uint32_t req_seq = protocol.get_next_sequence();
        std::vector<uint8_t> request_packet = protocol.pack(ATProtocol::FLAG_REQUEST, request_body, req_seq);


        // Send request
        if (send_all(sock, request_packet) != request_packet.size()) {
            spdlog::error("Failed to send request");
        }
        spdlog::debug("Sent request: {}", request_body);

        // --- Set socket timeout for receiving response ---
        struct timeval tv;
        tv.tv_sec = timeout_seconds;
        tv.tv_usec = 0;
        if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&tv), sizeof(tv)) == SOCKET_ERROR) {
            int err = get_last_error();
            spdlog::warn("Failed to set receive timeout: {}", get_error_message(err));
            // Continue anyway, might just block indefinitely on recv
        }

        // Receive response header first
        std::vector<uint8_t> header_buffer(ATProtocol::HEADER_SIZE);
        if (recv_all(sock, header_buffer, ATProtocol::HEADER_SIZE) != ATProtocol::HEADER_SIZE) {
            spdlog::error("Timeout or error receiving response header");
        }

        // Parse header to get body length
        ATHeader temp_header;
        // std::memcpy(&temp_header, header_buffer.data(), ATProtocol::HEADER_SIZE);
        temp_header.unpack(header_buffer.data(), header_buffer.size());
        uint32_t response_seq = temp_header.sequence;
        uint32_t body_len = temp_header.body_length;
        uint16_t response_flags = temp_header.flags;

        // Check sequence number
        if (response_seq != req_seq) {
            spdlog::warn("Sequence number mismatch: expected {}, got {}", response_seq, protocol.get_next_sequence() - 1);
            // Decide how strict to be. For now, proceed but log.
        }

        // Receive response body
        if (body_len < 0 || body_len > 10 * 1024 * 1024) { // Arbitrary 10MB limit
            spdlog::error("Unreasonable body length: {}", body_len);
            throw std::runtime_error("Invalid body length");
        }
        std::vector<uint8_t> body_buffer(body_len);
        if (recv_all(sock, body_buffer, body_len) != body_len) {
            spdlog::error("Error receiving response body");
        }

        // Combine for unpacking
        std::vector<uint8_t> full_response_packet;
        full_response_packet.insert(full_response_packet.end(), header_buffer.begin(), header_buffer.end());
        full_response_packet.insert(full_response_packet.end(), body_buffer.begin(), body_buffer.end());

        // Unpack response
        uint16_t received_flags;
        uint32_t received_seq;
        std::string response_body_str;
        protocol.unpack(full_response_packet, received_flags, received_seq, response_body_str);

        json response_json;
        try {
            response_json = json::parse(response_body_str);
        } catch (const json::exception& e) {
            spdlog::error("Failed to parse response JSON: {}", e.what());
            throw std::runtime_error("Invalid response format");
        }

        if (received_flags & ATProtocol::FLAG_ERROR || response_json.value("status", "") == "error") {
            std::string msg = response_json.value("message", "Unknown remote error");
            spdlog::error("Remote error: {}", msg);
            throw std::runtime_error("Remote Error: " + msg);
        }

        spdlog::debug("Received response: {}", response_body_str);

    } catch (const std::exception& e) {
        spdlog::error("Error during RPC call: {}", e.what());
        throw; // Re-throw to be caught by main
    } finally_block: // Simulate finally-like behavior (C++11 doesn't have try-finally)
        close_socket(sock);
        cleanup_sockets();
}

void run_server(const std::string& host, int port) {
    initialize_sockets();

    socket_t listen_sock = create_socket();
    if (listen_sock == INVALID_SOCKET) {
        spdlog::critical("Failed to create server socket.");
    }

    if (!bind_socket(listen_sock, host, port)) {
        spdlog::critical("Failed to bind server socket to {}:{}", host, port);
        close_socket(listen_sock);
        cleanup_sockets();
    }

    if (::listen(listen_sock, SOMAXCONN) == SOCKET_ERROR) {
        int err = get_last_error();
        spdlog::critical("Listen failed: {}", get_error_message(err));
        close_socket(listen_sock);
        cleanup_sockets();
    }

    spdlog::info("RPC Server listening on {}:{}",  host, port);

    while (true) {
        sockaddr_in client_addr{};
        socklen_t client_addr_len = sizeof(client_addr);
        socket_t client_socket = ::accept(listen_sock, (sockaddr*)&client_addr, &client_addr_len);
        if (client_socket == INVALID_SOCKET) {
           // Check if shutdown was intended
            int err = get_last_error();
            spdlog::error("Accept failed: {}", get_error_message(err));
            continue; // Continue loop even on accept failure, unless shutting down
        }
        spdlog::info("Accepted connection");

        // Handle each client
        std::vector <uint8_t> header_buffer(ATProtocol::HEADER_SIZE);
        if (recv_all(client_socket, header_buffer, ATProtocol::HEADER_SIZE) != ATProtocol::HEADER_SIZE) {
            spdlog::error("Failed to receive request header");
            close_socket(client_socket);
            continue;
        }
        // Parse header
        ATHeader header;
        header.unpack(header_buffer.data(), header_buffer.size());
        if (header.protocol_id != ATProtocol::PROTOCOL_ID) {
            spdlog::error("Invalid protocol ID: {}", header.protocol_id);
            close_socket(client_socket);
            continue;
        }
        if (header.body_length < 0 || header.body_length > 10 * 1024 * 1024) { // Arbitrary 10MB limit
            spdlog::error("Unreasonable body length: {}", header.body_length);
            close_socket(client_socket);
            continue;
        }
        if (!(header.flags & ATProtocol::FLAG_REQUEST)) { // Not a request
            spdlog::warn("Received non-request packet (Seq: {}, Flags: 0x{:04X}). Ignoring.", header.sequence, header.flags);
            continue; // Expecting a request
        }
        std::vector<uint8_t> body_buffer(header.body_length);
        if (recv_all(client_socket, body_buffer, header.body_length) != header.body_length) {
            spdlog::error("Failed to receive request body");
            close_socket(client_socket);
            continue;
        }

        std::vector<uint8_t> full_request_packet;
        full_request_packet.insert(full_request_packet.end(), header_buffer.begin(), header_buffer.end());
        full_request_packet.insert(full_request_packet.end(), body_buffer.begin(), body_buffer.end());

        try {
            ATProtocol protocol;
            uint16_t received_flags;
            uint32_t received_seq;
            std::string request_body_str;
            if (!protocol.unpack(full_request_packet, received_flags, received_seq, request_body_str)) {
                spdlog::error("Failed to unpack request");
                close_socket(client_socket);
                continue;
            }
            spdlog::debug("Received request: {}", request_body_str);
            json request = json::parse(request_body_str);
            json response_json = calculator_handler(request);
            std::string response_body_str = response_json.dump(); // Serialize response

            uint16_t response_flags = ATProtocol::FLAG_RESPONSE;
            if (response_json["status"] == "error") {
                response_flags |= ATProtocol::FLAG_ERROR;
            }
            auto packed_response = protocol.pack(response_flags, response_body_str, received_seq);
            if (send_all(client_socket, packed_response) != packed_response.size()) {
                spdlog::error("Failed to send response for seq={}", received_seq);
            }
            spdlog::info("Sent response (seq={}): {}", received_seq, response_body_str);
        
        } catch (const std::exception& e) {
            spdlog::error("Failed to process a message (seq={}): {}. Skipping it.", header.sequence, e.what());
            // buffer.clear();
            continue;
        }
        close_socket(client_socket);
    }
    close_socket(listen_sock);
    cleanup_sockets();
}

std::vector<double> parse_args(const std::string& args_str) {
    std::vector<double> args;
    std::stringstream ss(args_str);
    std::string item;
    while (std::getline(ss, item, ',')) {
        try {
            // Trim whitespace (basic)
            item.erase(0, item.find_first_not_of(' '));
            item.erase(item.find_last_not_of(' ') + 1);
            if(!item.empty()) {
                 args.push_back(std::stod(item));
            }
        } catch (const std::invalid_argument&) {
            throw std::invalid_argument("Invalid argument in list: " + item);
        } catch (const std::out_of_range&) {
             throw std::out_of_range("Argument out of range: " + item);
        }
    }
    return args;
}

int main(int argc, char* argv[]) {
    // Initialize logging early
    spdlog::set_level(spdlog::level::debug); // Set default level
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v"); // Basic pattern

    cxxopts::Options options("at_rpc_demo", "A simple RPC demo over TCP using a custom AT protocol.");

    options.add_options()
            // 位置参数，不带 '--' 前缀
            ("mode", "Run as 'server' or 'client'", cxxopts::value<std::string>())
            
            // 可选参数，带 '--' 前缀
            ("host,H", "Host to connect to or bind to", cxxopts::value<std::string>()->default_value("127.0.0.1"))
            ("port,p", "Port to connect to or bind to", cxxopts::value<int>()->default_value("9999"))
            ("func,f", "Function to call on the server (client mode only)", cxxopts::value<std::string>()->default_value("add"))
            ("args,a", "Arguments for the function (client mode only)", cxxopts::value<std::string>()->default_value("10,20"))
            ("help", "Print usage")
        ;

    // 告诉 cxxopts 'mode' 是一个位置参数
    options.parse_positional({"mode"});

    auto result = options.parse(argc, argv);

    // 如果用户请求帮助，打印并退出
    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        return 0;
    }
    // 检查必需的位置参数 'mode' 是否存在
    if (!result.count("mode")) {
        std::cerr << "Error: Mode argument is required." << std::endl;
        std::cout << options.help() << std::endl;
        return 1;
    }

     // 从解析结果中提取所有参数
    std::string mode = result["mode"].as<std::string>();

    std::string host = result["host"].as<std::string>();
    int port = result["port"].as<int>();

     if (mode == "server") {
        spdlog::info("Starting RPC Server...");
        run_server(host, port);
    } else if (mode == "client") {
        if (!result.count("func") || !result.count("args")) {
            std::cerr << "Error: Client mode requires --func and --args." << std::endl;
            std::cout << options.help({"Client"}) << std::endl;
            return 1;
        }
        std::string func = result["func"].as<std::string>();
        std::string args_str = result["args"].as<std::string>();
        std::vector<double> args;
        try{
            args = parse_args(args_str);
        } catch (const std::exception& e) {
            std::cerr << "Error parsing arguments: " << e.what() << std::endl;
            return 1;
        }
        std::cout << "Calling remote function '" << func << ", args {"<< args_str <<"}"<<std::endl;
        spdlog::info("Running RPC Client...");
        run_client(host, port, func, args);
    }

    return 0;
}

// run: .\build\at_rpc_demo server --host 127.0.0.1 --port 9999
// run: .\build\at_rpc_demo client --host 127.0.0.1 --port 9999 --func add --args 10,20