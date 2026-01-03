// rpc_client.cpp
#include "network_utils.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>
#include "at_protocol.h"

json call_remote_function(const std::string& host, int port, const std::string& func, const std::vector<double>& args, int timeout_seconds = 10) {

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
        auto request_packet = protocol.pack(ATProtocol::FLAG_REQUEST, request_body);


        // Send request
        if (send_all(sock, request_packet) != request_packet.size()) {
            throw std::runtime_error("Failed to send request");
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
            throw std::runtime_error("Timeout or error receiving response header");
        }

        // Parse header to get body length
        ATHeader temp_header;
        std::memcpy(&temp_header, header_buffer.data(), ATProtocol::HEADER_SIZE);

        uint32_t response_seq = temp_header.sequence;
        uint32_t body_len = temp_header.body_length;
        uint16_t response_flags = temp_header.flags;

        // Check sequence number
        if (response_seq != protocol.get_next_sequence() - 1) {
            spdlog::warn("Sequence number mismatch: expected {}, got {}", response_seq, protocol.get_next_sequence() - 1);
            // Decide how strict to be. For now, proceed but log.
        }

        // Receive response body
        std::vector<uint8_t> body_buffer(body_len);
        if (recv_all(sock, body_buffer, body_len) != body_len) {
            throw std::runtime_error("Error receiving response body");
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
        return response_json;

    } catch (const std::exception& e) {
        spdlog::error("Error during RPC call: {}", e.what());
        throw; // Re-throw to be caught by main
    } finally_block: // Simulate finally-like behavior (C++11 doesn't have try-finally)
        close_socket(sock);
        cleanup_sockets();
        // Note: goto finally_block is generally discouraged, but shows intent clearly here.
        // Better handled by RAII or structured exception handling if more complex resource management is needed.
}


extern void run_client(const std::string& host, int port, const std::string& func, const std::vector<double>& args) {
    try {
        json result = call_remote_function(host, port, func, args);
        if (result.contains("result")) {
            std::cout << "Result: " << result["result"] << std::endl;
        } else {
            std::cerr << "Unexpected response format." << std::endl;
            std::cerr << result.dump(4) << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Client error: " << e.what() << std::endl;
        exit(EXIT_FAILURE); // Exit with error code
    }
}