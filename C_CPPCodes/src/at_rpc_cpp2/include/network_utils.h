// network_utils.h
#pragma once

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "Ws2_32.lib") // Automatically link Winsock library
    typedef SOCKET socket_t;
    // #define INVALID_SOCKET INVALID_SOCKET
    // #define SOCKET_ERROR SOCKET_ERROR
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <errno.h>
    #include <string.h> // For strerror
    typedef int socket_t;
    #define INVALID_SOCKET (-1)
    #define SOCKET_ERROR (-1)
#endif

#include <string>
#include <vector>
#include <cstdint>

// Function declarations
void initialize_sockets();
void cleanup_sockets();
socket_t create_socket();
bool bind_socket(socket_t sock, const std::string& host, int port);
bool connect_socket(socket_t sock, const std::string& host, int port);
void close_socket(socket_t sock);
int get_last_error();
std::string get_error_message(int err);

size_t send_all(socket_t sockfd, const std::vector<uint8_t>& data);
size_t recv_all(socket_t sockfd, std::vector<uint8_t>& data, size_t len);