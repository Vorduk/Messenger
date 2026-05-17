#include "TcpClient.h"
#include <iostream>
#include <cstring>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

TcpClient::TcpClient(const std::string& server_address, int port)
    : m_server_address(server_address), m_port(port) {
#ifdef _WIN32
    WSADATA wsaData;    // Struct for Winsock.
    WSAStartup(MAKEWORD(2, 2), &wsaData);   // Initialize Winsock 2.2.
#endif
}

TcpClient::~TcpClient() {
    if (m_socket != -1) {
#ifdef _WIN32
        closesocket(m_socket);
        WSACleanup();
#else
        close(m_socket);
#endif
    }
}

bool TcpClient::connectToServer() {
    // Socket creation with IPv4 address family
#ifdef _WIN32
    m_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socket == INVALID_SOCKET) return false;
#else
    m_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socket < 0) return false;
#endif

    struct sockaddr_in addr; // Server address structure.
    addr.sin_family = AF_INET; // IPv4.
    addr.sin_port = htons(m_port); // Port (Host TO Network Short).
    inet_pton(AF_INET, m_server_address.c_str(), &addr.sin_addr); // Ip to binary.

    // Tcp connection.
    if (connect(m_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
#ifdef _WIN32
        closesocket(m_socket);
#else
        close(m_socket);
#endif
        m_socket = -1;
        return false;
    }
    m_is_server_connected = true;
    return true;
}

std::string TcpClient::sendRequest(const std::string& request_json) {
    if (!m_is_server_connected) return R"({"status":"error","message":"Not connected"})"; // Return json string with error.

    std::string data = request_json + "\n"; // Mark the end of message.

    // Send data.
#ifdef _WIN32
    send(m_socket, data.c_str(), data.size(), 0);
#else
    send(m_socket, data.c_str(), data.size(), 0);
#endif

    // Get response.
    std::string response;
    char buf[4096];
    while (true) {
        memset(buf, 0, sizeof(buf));
#ifdef _WIN32
        int bytes_readed = recv(m_socket, buf, sizeof(buf) - 1, 0);
#else
        int bytes_readed = recv(m_socket, buf, sizeof(buf) - 1, 0);
#endif
        if (bytes_readed <= 0) break; // Server closed connection.
        response.append(buf, bytes_readed); // Add new bytes to response string
        if (response.find('\n') != std::string::npos) break; // Response fully transfered, end loop.
    }

    if (!response.empty() && response.back() == '\n') response.pop_back(); // Delete "\n", doesn't need in Json parsing.
    return response;
}