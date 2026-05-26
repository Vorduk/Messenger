#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <thread>
#include <atomic>
#include <sqlite3.h>
#include <mutex>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <openssl/ssl.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

class TestServer {
public:
    TestServer(int port, const std::string& dbPath, const std::string& cert_file, const std::string& key_file);
    ~TestServer();

    void run();
    void stop();

private:
    void clientHandler(int client_fd);
    std::pair<std::string, std::string> processRequest(const std::string& json);

    // Database operations
    bool registerUser(const std::string& username, const std::string& display_name, const std::string& birthday, std::string& outUserId, std::string& outDisplayName, std::string& outError);
    bool loginUser(const std::string& username, std::string& outUserId, std::string& outDisplayName, std::string& outError);
    std::string getUsers(const std::string& excludeUserId);
    bool sendMessage(const std::string& senderId, const std::string& receiverId, const std::string& text, std::string& outMessageId, long long& outTimestamp);
    std::string getMessages(const std::string& user1, const std::string& user2, int limit, int offset);
    std::string getChats(const std::string& userId); 
    std::string searchUsers(const std::string& excludeUserId, const std::string& query);

    bool createTables();
    static std::string generateId();

    int m_listenFd;
    sqlite3* m_db;
    std::atomic<bool> m_running{ false };
    std::vector<std::thread> m_threads;
    std::unordered_map<std::string, int> m_user_sockets;
    std::mutex m_user_sockets_mutex;

    SSL_CTX* m_ssl_ctx = nullptr;
    bool m_use_tls = false;
};