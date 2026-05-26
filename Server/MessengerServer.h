#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <thread>
#include <atomic>
#include <sqlite3.h>
#include <mutex>
#include <openssl/ssl.h>
#include <json.hpp>

// Platform-specific socket headers
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

/**
 * @brief The main chat server that handles client connections, user authentication,
 * and message exchange. Supports optional TLS encryption.
 */
class MessengerServer {
public:
    /**
     * @brief Constructs the server, initialises the listening socket and database.
     * @param port       TCP port to listen on.
     * @param db_path    Path to the SQLite database file.
     * @param cert_file  Path to the TLS certificate file (PEM format). Leave empty to disable TLS.
     * @param key_file   Path to the TLS private key file (PEM format). Leave empty to disable TLS.
     */
    MessengerServer(int port, const std::string& db_path,
        const std::string& cert_file, const std::string& key_file);
    ~MessengerServer();

    /**
     * @brief Starts the main accept loop. Blocks until stop() is called.
     */
    void run();

    /**
     * @brief Signals the server to stop accepting new connections and waits for existing threads to finish.
     */
    void stop();

private:
    // Initialisation helpers
    bool initWinsock();
    bool initDatabase(const std::string& db_path);
    bool initSocket(int port);
    bool initTls(const std::string& cert_file, const std::string& key_file);
    void cleanup();

    // Client handling
    int  acceptClient();
    void clientHandler(int client_fd);

    SSL* performTlsAccept(int client_fd);
    int  readData(int client_fd, SSL* ssl, char* buffer, int size);
    int  sendData(int client_fd, SSL* ssl, const char* data, int len);
    void handleClientDisconnect(int client_fd, SSL* ssl, const std::string& user_id);

    // User online status tracking
    void registerUserSocket(const std::string& user_id, int client_fd);
    void unregisterUserSocket(const std::string& user_id, int client_fd);

    // Request processing
    std::pair<std::string, std::string> processRequest(const std::string& json);
    // Individual action handlers (return JSON response and optional authenticated user ID)
    std::pair<nlohmann::json, std::string> handleRegister(const nlohmann::json& req);
    std::pair<nlohmann::json, std::string> handleLogin(const nlohmann::json& req);
    nlohmann::json handleGetUsers(const nlohmann::json& req);
    nlohmann::json handleSendMessage(const nlohmann::json& req);
    nlohmann::json handleGetMessages(const nlohmann::json& req);
    nlohmann::json handleGetChats(const nlohmann::json& req);
    nlohmann::json handleSearchUsers(const nlohmann::json& req);

    // Database operations (wrappers around SQLite)
    bool createTables();
    static std::string generateId();

    bool dbRegisterUser(const std::string& username, const std::string& display_name,
        const std::string& birthday,
        std::string& out_user_id, std::string& out_display_name,
        std::string& out_error);
    bool dbLoginUser(const std::string& username,
        std::string& out_user_id, std::string& out_display_name,
        std::string& out_error);
    std::string dbGetUsers(const std::string& exclude_user_id);
    bool dbSendMessage(const std::string& sender_id, const std::string& receiver_id,
        const std::string& text, std::string& out_message_id,
        long long& out_timestamp);
    std::string dbGetMessages(const std::string& user1, const std::string& user2,
        int limit, int offset);
    std::string dbGetChats(const std::string& user_id);
    std::string dbSearchUsers(const std::string& exclude_user_id, const std::string& query);

    // Helper to append last message info to a JSON user/chat object
    void appendLastMessageJson(nlohmann::json& target,
        const std::string& current_user_id,
        const std::string& partner_id);

    // Member variables
    int m_listen_fd = -1;
    sqlite3* m_db = nullptr;
    std::atomic<bool> m_running{ false };
    std::vector<std::thread> m_threads;
    std::unordered_map<std::string, int> m_user_sockets;
    std::mutex m_user_sockets_mutex;

    SSL_CTX* m_ssl_ctx = nullptr;
    bool m_use_tls = false;
};