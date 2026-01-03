// network_utils.cpp
#include "network_utils.h"
#include <spdlog/spdlog.h>
#include <stdexcept>


// --- Utility functions for sending/receiving all data ---
size_t send_all(socket_t sockfd, const std::vector<uint8_t>& data) {
    size_t total_sent = 0;
    const char* buf = reinterpret_cast<const char*>(data.data());
    size_t len = data.size();
    while (total_sent < len) {
        spdlog::debug("Attempting to send {} bytes...", len - total_sent);
        int sent = ::send(sockfd, buf + total_sent, static_cast<int>(len - total_sent), 0);
        if (sent == SOCKET_ERROR) {
            int err = get_last_error();
            spdlog::error("send failed: {}", get_error_message(err));
        }
        if (sent == 0) {
            spdlog::warn("Connection closed by peer during send.");
        }
        total_sent += sent;
    }
    spdlog::debug("Successfully sent {} bytes.", total_sent); // 添加成功发送日志
    return total_sent;
}

size_t recv_all(socket_t sockfd, std::vector<uint8_t>& data, size_t len) {
    char* buf = reinterpret_cast<char*>(data.data());
    size_t total_received = 0;
    while (total_received < len) {
        spdlog::debug("Attempting to recv {} bytes...", len - total_received);
        int received = ::recv(sockfd, buf + total_received, static_cast<int>(len - total_received), 0);
        if (received == SOCKET_ERROR) {
            int err = get_last_error();
            spdlog::error("recv failed: {}", get_error_message(err));
        }
        if (received == 0) {
            spdlog::info("Connection closed by peer during receive.");
        }
        total_received += received;
    }
    return total_received;
}

#ifdef _WIN32
void initialize_sockets() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        spdlog::critical("WSAStartup failed: {}", result);
        throw std::runtime_error("WSAStartup failed");
    }
    spdlog::debug("Winsock initialized.");
}

void cleanup_sockets() {
    WSACleanup();
    spdlog::debug("Winsock cleaned up.");
}

socket_t create_socket() {
    socket_t sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        int err = WSAGetLastError();
        spdlog::error("Socket creation failed: {}", err);
    }
    return sock;
}

bool bind_socket(socket_t sock, const std::string& host, int port) {
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<u_short>(port));
    if (host == "0.0.0.0") {
        addr.sin_addr.s_addr = INADDR_ANY;
    } else {
        inet_pton(AF_INET, host.c_str(), &addr.sin_addr);
    }

    if (::bind(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR) {
        int err = WSAGetLastError();
        spdlog::error("Bind failed: {}", err);
        return false;
    }
    return true;
}

bool connect_socket(socket_t sock, const std::string& host, int port) {
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<u_short>(port));
    inet_pton(AF_INET, host.c_str(), &addr.sin_addr);

    if (::connect(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR) {
        int err = WSAGetLastError();
        spdlog::error("Connect failed: {}", err);
        return false;
    }
    return true;
}

void close_socket(socket_t sock) {
    closesocket(sock); // Win32
}

int get_last_error() {
    return WSAGetLastError();
}

std::string get_error_message(int err) {
    char* s = nullptr;
    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                   NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&s, 0, NULL);
    std::string msg(s ? s : "Unknown error");
    LocalFree(s);
    return msg;
}

#else // Unix/Linux

#include <fcntl.h> // For fcntl (making socket non-blocking potentially)

void initialize_sockets() {
    // No explicit initialization needed on Unix
    spdlog::debug("POSIX sockets initialized (no-op).");
}

void cleanup_sockets() {
    // No explicit cleanup needed on Unix
    spdlog::debug("POSIX sockets cleaned up (no-op).");
}

socket_t create_socket() {
    socket_t sock = ::socket(AF_INET, SOCK_STREAM, 0); // IPPROTO_TCP is 0 for SOCK_STREAM
    if (sock == INVALID_SOCKET) {
        spdlog::error("Socket creation failed: {}", strerror(errno));
    }
    return sock;
}

bool bind_socket(socket_t sock, const std::string& host, int port) {
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<uint16_t>(port));
    if (host == "0.0.0.0") {
        addr.sin_addr.s_addr = INADDR_ANY;
    } else {
        inet_pton(AF_INET, host.c_str(), &addr.sin_addr);
    }

    if (::bind(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR) {
        spdlog::error("Bind failed: {}", strerror(errno));
        return false;
    }
    return true;
}

bool connect_socket(socket_t sock, const std::string& host, int port) {
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<uint16_t>(port));
    inet_pton(AF_INET, host.c_str(), &addr.sin_addr);

    if (::connect(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR) {
       spdlog::error("Connect failed: {}", strerror(errno));
       return false;
    }
    return true;
}

void close_socket(socket_t sock) {
    ::close(sock); // POSIX
}

int get_last_error() {
    return errno;
}

std::string get_error_message(int err) {
    return strerror(err);
}

#endif