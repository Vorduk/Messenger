#include "pch.h"
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

// Call one time at app start.
void initializeOpenSSL() {
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
}

TcpClient::TcpClient(const std::string& server_address, int port)
    : m_server_address(server_address), m_port(port) {
#ifdef _WIN32
    WSADATA wsaData;    // Struct for Winsock.
    WSAStartup(MAKEWORD(2, 2), &wsaData);   // Initialize Winsock 2.2.
#endif
}

TcpClient::~TcpClient() {
    cleanupSsl();
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
    m_socket = socket(AF_INET, SOCK_STREAM, 0); // IpV4; stream socket; TCP.
    if (m_socket < 0) return false; // No sockets left or incorrect params were passed to function.
#endif

    struct sockaddr_in addr; // Server address structure.
    addr.sin_family = AF_INET; // IPv4.
    addr.sin_port = htons(m_port); // Port (Host TO Network Short, little endian to big endian).
    inet_pton(AF_INET, m_server_address.c_str(), &addr.sin_addr); // Ip to bytes.

    // Tcp connection. 3-way handshake.
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

    setTimeouts(5000, 5000);

    if (m_use_tls) {
        m_ssl = SSL_new(m_ssl_ctx);
        SSL_set_fd(m_ssl, m_socket);
        if (SSL_connect(m_ssl) <= 0) {
            ERR_print_errors_fp(stderr);
            cleanupSsl();
            closeSocket();
            return false;
        }
    }

    return true;
}

std::string TcpClient::sendRequest(const std::string& request_json) {
    // Return an error immediately if the client is not connected.
    if (!m_is_server_connected)
        return R"({"status":"error","message":"Not connected"})";

    // Append a newline character to mark the end of the request.
    std::string data = request_json + "\n";

    // Send the request data.
    // For TLS connections we use SSL_write(); otherwise we use plain send().
    int total_sent = 0;
    int len = static_cast<int>(data.size());
    while (total_sent < len) {
        int sent = 0;
        if (m_use_tls) {
            // SSL_write returns the number of bytes written, or <= 0 on error.
            sent = SSL_write(m_ssl, data.c_str() + total_sent, len - total_sent);
        }
        else {
            // On Windows MSG_NOSIGNAL is not defined; use 0 instead.
            // On Linux MSG_NOSIGNAL prevents the SIGPIPE signal if the socket is broken.
#ifdef _WIN32
            sent = send(m_socket, data.c_str() + total_sent, len - total_sent, 0);
#else
            sent = send(m_socket, data.c_str() + total_sent, len - total_sent, MSG_NOSIGNAL);
#endif
        }

        // If send fails, disconnect and return an error response.
        if (sent <= 0) {
            disconnect();
            return R"({"status":"error","message":"Send failed, disconnected"})";
        }
        total_sent += sent;
    }

    // Receive the response.
    // For TLS we use SSL_read(); otherwise we use plain recv().
    std::string response;
    char buf[4096];
    while (true) {
        memset(buf, 0, sizeof(buf));
        int bytes_read = 0;
        if (m_use_tls) {
            // SSL_read returns the number of bytes read, or <= 0 on error/EOF.
            bytes_read = SSL_read(m_ssl, buf, sizeof(buf) - 1);
        }
        else {
#ifdef _WIN32
            bytes_read = recv(m_socket, buf, sizeof(buf) - 1, 0);
#else
            bytes_read = recv(m_socket, buf, sizeof(buf) - 1, 0);
#endif
        }

        if (bytes_read <= 0) {
            // Connection closed or error occurred.
            m_is_server_connected = false;
            closeSocket();
            return R"({"status":"error","message":"Server disconnected"})";
        }

        // Append received data to the leftover buffer.
        m_leftover_buffer.append(buf, bytes_read);

        // If a complete line (terminated by '\n') is found, stop reading.
        if (m_leftover_buffer.find('\n') != std::string::npos) {
            break;
        }
    }

    // Extract the first complete JSON line from the buffer.
    size_t newline_pos = m_leftover_buffer.find('\n');
    if (newline_pos == std::string::npos) {
        // This should not happen because we already checked for '\n'.
        return R"({"status":"error","message":"Invalid server response"})";
    }

    response = m_leftover_buffer.substr(0, newline_pos);

    // Remove the used line (including the newline) from the buffer,
    // leaving any remaining data for the next read.
    m_leftover_buffer.erase(0, newline_pos + 1);

    return response;
}

bool TcpClient::isConnected() const
{
    return m_is_server_connected;
}

void TcpClient::setTimeouts(int send_timeout_ms, int recv_timeout_ms) {
    if (m_socket != -1) {
#ifdef _WIN32
        DWORD timeout = send_timeout_ms;
        setsockopt(m_socket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout));
        timeout = recv_timeout_ms;
        setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
#else
        struct timeval tv;
        tv.tv_sec = send_timeout_ms / 1000;
        tv.tv_usec = (send_timeout_ms % 1000) * 1000;
        setsockopt(m_socket, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
        tv.tv_sec = recv_timeout_ms / 1000;
        tv.tv_usec = (recv_timeout_ms % 1000) * 1000;
        setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
#endif
        m_has_timeouts = true;
    }
}

void TcpClient::disconnect() {
    m_is_server_connected = false;
    closeSocket();
}

void TcpClient::enableTls() {
    m_use_tls = true;
    m_ssl_ctx = SSL_CTX_new(TLS_client_method());
    // For development only – accept any certificate.
    SSL_CTX_set_verify(m_ssl_ctx, SSL_VERIFY_NONE, nullptr);
}

void TcpClient::closeSocket() {
    if (m_socket != -1) {
#ifdef _WIN32
        closesocket(m_socket);
#else
        close(m_socket);
#endif
        m_socket = -1;
    }
}

void TcpClient::cleanupSsl() {
    if (m_ssl_ctx) {
        SSL_CTX_free(m_ssl_ctx);
        m_ssl_ctx = nullptr;
    }
}