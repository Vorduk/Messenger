#include "MessageSender.h"
#include <iostream>
#include <cstring>

#ifdef _WIN32
#define _WIN32_WINNT 0x0600  // для inet_pton
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

// Статическая инициализация Winsock (только для Windows)
#ifdef _WIN32
static bool initWinsock() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return false;
    }
    return true;
}

static void cleanupWinsock() {
    WSACleanup();
}
#endif

MessageSender::MessageSender(const std::string& host, int port)
    : m_socket(-1), m_host(host), m_port(port), m_is_connected(false)
{
#ifdef _WIN32
    static bool winsockInitialized = initWinsock();
    (void)winsockInitialized; // подавляем предупреждение
#endif
}

MessageSender::~MessageSender() {
    if (m_socket != -1) {
#ifdef _WIN32
        closesocket(m_socket);
#else
        close(m_socket);
#endif
    }
#ifdef _WIN32
    static bool cleanupDone = [] { cleanupWinsock(); return true; }();
    (void)cleanupDone;
#endif
}

bool MessageSender::connect() {
#ifdef _WIN32
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Socket creation error" << std::endl;
        return false;
    }
#else
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Socket creation error" << std::endl;
        return false;
    }
#endif
    m_socket = sock;

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(m_port);
#ifdef _WIN32
    if (inet_pton(AF_INET, m_host.c_str(), &serv_addr.sin_addr) != 1) {
        std::cerr << "Invalid address" << std::endl;
        closesocket(m_socket);
        m_socket = -1;
        return false;
    }
#else
    if (inet_pton(AF_INET, m_host.c_str(), &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address" << std::endl;
        close(m_socket);
        m_socket = -1;
        return false;
    }
#endif

    if (::connect(m_socket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection failed: " << std::strerror(errno) << std::endl;
#ifdef _WIN32
        closesocket(m_socket);
#else
        close(m_socket);
#endif
        m_socket = -1;
        m_is_connected = false;
        return false;
    }

    m_is_connected = true;
    return true;
}

bool MessageSender::send(const std::string& serialized_message) {
    if (!m_is_connected) {
        std::cerr << "Not connected" << std::endl;
        return false;
    }
    std::string data = serialized_message + "\n";
#ifdef _WIN32
    int result = ::send(m_socket, data.c_str(), static_cast<int>(data.size()), 0);
#else
    int result = ::send(m_socket, data.c_str(), data.size(), 0);
#endif
    return result != -1;
}
