#include "pch.h"
#include "MessengerServer.h"
#include <iostream>
#include <cstring>
#include "LogMacros.h"
#include "UUIDGenerator.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

// Convenience macro for closing sockets cross‑platform
#ifdef _WIN32
#define CLOSE_SOCKET(s) closesocket(s)
#else
#define CLOSE_SOCKET(s) close(s)
#endif

// Constructor / Destructor

MessengerServer::MessengerServer(int port, const std::string& db_path,
    const std::string& cert_file, const std::string& key_file) {
    if (!initWinsock())   return;
    if (!initDatabase(db_path)) return;
    if (!initSocket(port)) return;
    LOG_INFO("Server listening on port {}", port);
    if (!cert_file.empty() && !key_file.empty()) {
        if (!initTls(cert_file, key_file)) {
            exit(1);
        }
    }
}

MessengerServer::~MessengerServer() {
    stop();
    CLOSE_SOCKET(m_listen_fd);
#ifdef _WIN32
    WSACleanup();
#endif
    if (m_db) sqlite3_close(m_db);
    if (m_ssl_ctx) SSL_CTX_free(m_ssl_ctx);
}


// Initialisation helpers

bool MessengerServer::initWinsock() {
#ifdef _WIN32
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        LOG_ERROR("WSAStartup failed");
        return false;
    }
#endif
    return true;
}

bool MessengerServer::initDatabase(const std::string& db_path) {
    if (sqlite3_open(db_path.c_str(), &m_db) != SQLITE_OK) {
        LOG_ERROR("Cannot open database: {}", sqlite3_errmsg(m_db));
        return false;
    }
    return createTables();
}

bool MessengerServer::initSocket(int port) {
#ifdef _WIN32
    m_listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_listen_fd == INVALID_SOCKET) {
        LOG_ERROR("Socket creation failed");
        return false;
    }
#else
    m_listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_listen_fd < 0) {
        LOG_ERROR("Socket creation failed");
        return false;
    }
#endif
    int opt = 1;
    setsockopt(m_listen_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

#ifdef _WIN32
    if (bind(m_listen_fd, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
        LOG_ERROR("Bind failed");
        CLOSE_SOCKET(m_listen_fd);
        return false;
    }
    if (listen(m_listen_fd, SOMAXCONN) == SOCKET_ERROR) {
        LOG_ERROR("Listen failed");
        CLOSE_SOCKET(m_listen_fd);
        return false;
    }
#else
    if (bind(m_listen_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        LOG_ERROR("Bind failed");
        close(m_listen_fd);
        return false;
    }
    if (listen(m_listen_fd, SOMAXCONN) < 0) {
        LOG_ERROR("Listen failed");
        close(m_listen_fd);
        return false;
    }
#endif
    return true;
}

bool MessengerServer::initTls(const std::string& cert_file, const std::string& key_file) {
    m_ssl_ctx = SSL_CTX_new(TLS_server_method());
    SSL_CTX_set_ecdh_auto(m_ssl_ctx, 1);
    if (SSL_CTX_use_certificate_file(m_ssl_ctx, cert_file.c_str(), SSL_FILETYPE_PEM) <= 0 ||
        SSL_CTX_use_PrivateKey_file(m_ssl_ctx, key_file.c_str(), SSL_FILETYPE_PEM) <= 0) {
        LOG_ERROR("Failed to load certificate/key");
        return false;
    }
    m_use_tls = true;
    return true;
}


// Run / Stop

void MessengerServer::run() {
    m_running = true;
    while (m_running) {
        int client_fd = acceptClient();
        if (client_fd < 0) {
            if (m_running) LOG_ERROR("Accept failed");
            continue;
        }
        LOG_INFO("New client connected");
        m_threads.emplace_back(&MessengerServer::clientHandler, this, client_fd);
    }
}

void MessengerServer::stop() {
    m_running = false;
    // Join all client threads
    for (auto& t : m_threads) {
        if (t.joinable()) t.join();
    }
    m_threads.clear();
}

int MessengerServer::acceptClient() {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
#ifdef _WIN32
    return accept(m_listen_fd, (struct sockaddr*)&client_addr, &client_len);
#else
    return accept(m_listen_fd, (struct sockaddr*)&client_addr, &client_len);
#endif
}


// Client handler

void MessengerServer::clientHandler(int client_fd) {
    SSL* ssl = nullptr;
    if (m_use_tls) {
        ssl = performTlsAccept(client_fd);
        if (!ssl) {
            CLOSE_SOCKET(client_fd);
            return;
        }
    }

    std::string buffer;
    char tmp[4096];
    std::string current_user_id;

    while (m_running) {
        memset(tmp, 0, sizeof(tmp));
        int bytes = readData(client_fd, ssl, tmp, sizeof(tmp) - 1);
        if (bytes <= 0) {
            LOG_INFO("Client disconnected");
            break;
        }
        buffer.append(tmp, bytes);

        size_t pos;
        while ((pos = buffer.find('\n')) != std::string::npos) {
            std::string request = buffer.substr(0, pos);
            buffer.erase(0, pos + 1);

            auto [response, user_id] = processRequest(request);
            response += "\n";

            sendData(client_fd, ssl, response.c_str(), static_cast<int>(response.size()));

            if (!user_id.empty()) {
                current_user_id = user_id;
                registerUserSocket(user_id, client_fd);
            }
        }
    }

    handleClientDisconnect(client_fd, ssl, current_user_id);
}

SSL* MessengerServer::performTlsAccept(int client_fd) {
    SSL* ssl = SSL_new(m_ssl_ctx);
    SSL_set_fd(ssl, client_fd);
    if (SSL_accept(ssl) <= 0) {
        LOG_ERROR("SSL_accept failed");
        SSL_free(ssl);
        return nullptr;
    }
    return ssl;
}

int MessengerServer::readData(int client_fd, SSL* ssl, char* buffer, int size) {
    if (ssl)
        return SSL_read(ssl, buffer, size);
#ifdef _WIN32
    return recv(client_fd, buffer, size, 0);
#else
    return recv(client_fd, buffer, size, 0);
#endif
}

int MessengerServer::sendData(int client_fd, SSL* ssl, const char* data, int len) {
    if (ssl)
        return SSL_write(ssl, data, len);
#ifdef _WIN32
    return send(client_fd, data, len, 0);
#else
    return send(client_fd, data, len, 0);
#endif
}

void MessengerServer::handleClientDisconnect(int client_fd, SSL* ssl, const std::string& user_id) {
    if (!user_id.empty()) {
        unregisterUserSocket(user_id, client_fd);
    }
    if (ssl) {
        SSL_shutdown(ssl);
        SSL_free(ssl);
    }
    CLOSE_SOCKET(client_fd);
}


// User online status tracking

void MessengerServer::registerUserSocket(const std::string& user_id, int client_fd) {
    std::lock_guard<std::mutex> lock(m_user_sockets_mutex);
    m_user_sockets[user_id] = client_fd;
}

void MessengerServer::unregisterUserSocket(const std::string& user_id, int client_fd) {
    std::lock_guard<std::mutex> lock(m_user_sockets_mutex);
    auto it = m_user_sockets.find(user_id);
    if (it != m_user_sockets.end() && it->second == client_fd) {
        m_user_sockets.erase(it);
        // Reset the online status in the database
        const char* sql = "UPDATE users SET is_online = 0 WHERE id = ?;";
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, user_id.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
        }
    }
}

// Request processing

std::pair<std::string, std::string> MessengerServer::processRequest(const std::string& json) {
    nlohmann::json req;
    try {
        req = nlohmann::json::parse(json);
    }
    catch (...) {
        return { R"({"status":"error","message":"Invalid JSON"})", "" };
    }

    std::string action = req.value("action", "");
    nlohmann::json resp;
    std::string auth_user_id;

    if (action == "register") {
        std::tie(resp, auth_user_id) = handleRegister(req);
    }
    else if (action == "login") {
        std::tie(resp, auth_user_id) = handleLogin(req);
    }
    else if (action == "get_users") {
        resp = handleGetUsers(req);
    }
    else if (action == "send_message") {
        resp = handleSendMessage(req);
    }
    else if (action == "get_messages") {
        resp = handleGetMessages(req);
    }
    else if (action == "get_chats") {
        resp = handleGetChats(req);
    }
    else if (action == "search_users") {
        resp = handleSearchUsers(req);
    }
    else {
        resp["status"] = "error";
        resp["message"] = "Unknown action";
    }

    return { resp.dump(), auth_user_id };
}

// Individual action handlers

std::pair<nlohmann::json, std::string> MessengerServer::handleRegister(const nlohmann::json& req) {
    std::string username = req.value("username", "");
    std::string display_name = req.value("display_name", "");
    std::string birthday = req.value("birthday", "");
    std::string user_id, out_display_name, error;

    nlohmann::json resp;
    std::string auth_id;
    if (dbRegisterUser(username, display_name, birthday, user_id, out_display_name, error)) {
        resp["status"] = "ok";
        resp["user_id"] = user_id;
        resp["display_name"] = out_display_name;
        auth_id = user_id;
    }
    else {
        resp["status"] = "error";
        resp["message"] = error;
    }
    return { resp, auth_id };
}

std::pair<nlohmann::json, std::string> MessengerServer::handleLogin(const nlohmann::json& req) {
    std::string username = req.value("username", "");
    std::string user_id, out_display_name, error;

    nlohmann::json resp;
    std::string auth_id;
    if (dbLoginUser(username, user_id, out_display_name, error)) {
        resp["status"] = "ok";
        resp["user_id"] = user_id;
        resp["display_name"] = out_display_name;
        auth_id = user_id;
    }
    else {
        resp["status"] = "error";
        resp["message"] = error;
    }
    return { resp, auth_id };
}

nlohmann::json MessengerServer::handleGetUsers(const nlohmann::json& req) {
    std::string user_id = req.value("user_id", "");
    nlohmann::json resp;
    resp["status"] = "ok";
    resp["users"] = nlohmann::json::parse(dbGetUsers(user_id));
    return resp;
}

nlohmann::json MessengerServer::handleSendMessage(const nlohmann::json& req) {
    std::string sender_id = req.value("sender_id", "");
    std::string receiver_id = req.value("receiver_id", "");
    std::string text = req.value("text", "");
    std::string msg_id;
    long long timestamp;

    nlohmann::json resp;
    if (dbSendMessage(sender_id, receiver_id, text, msg_id, timestamp)) {
        resp["status"] = "ok";
        resp["message_id"] = msg_id;
        resp["timestamp"] = timestamp;
    }
    else {
        resp["status"] = "error";
        resp["message"] = "Failed to send message";
    }
    return resp;
}

nlohmann::json MessengerServer::handleGetMessages(const nlohmann::json& req) {
    std::string user_id = req.value("user_id", "");
    std::string partner_id = req.value("partner_id", "");
    int limit = req.value("limit", 50);
    int offset = req.value("offset", 0);

    nlohmann::json resp;
    resp["status"] = "ok";
    resp["messages"] = nlohmann::json::parse(dbGetMessages(user_id, partner_id, limit, offset));
    return resp;
}

nlohmann::json MessengerServer::handleGetChats(const nlohmann::json& req) {
    std::string user_id = req.value("user_id", "");
    nlohmann::json resp;
    resp["status"] = "ok";
    resp["chats"] = nlohmann::json::parse(dbGetChats(user_id));
    return resp;
}

nlohmann::json MessengerServer::handleSearchUsers(const nlohmann::json& req) {
    std::string user_id = req.value("user_id", "");
    std::string query = req.value("query", "");
    nlohmann::json resp;
    resp["status"] = "ok";
    resp["users"] = nlohmann::json::parse(dbSearchUsers(user_id, query));
    return resp;
}


// Database operations

bool MessengerServer::createTables() {
    const char* users_tbl = R"(
        CREATE TABLE IF NOT EXISTS users (
            id TEXT PRIMARY KEY,
            username TEXT UNIQUE NOT NULL,
            display_name TEXT NOT NULL,
            bio TEXT DEFAULT '',
            avatar_path TEXT DEFAULT '',
            birthday TEXT DEFAULT '',
            is_online INTEGER DEFAULT 0,
            show_online INTEGER DEFAULT 1,
            show_typing INTEGER DEFAULT 1
        );
    )";
    const char* msgs_tbl = R"(
        CREATE TABLE IF NOT EXISTS messages (
            id TEXT PRIMARY KEY,
            sender_id TEXT NOT NULL,
            receiver_id TEXT NOT NULL,
            text TEXT NOT NULL,
            timestamp INTEGER NOT NULL,
            status INTEGER DEFAULT 1
        );
    )";
    const char* idx1 = "CREATE INDEX IF NOT EXISTS idx_msg_ts ON messages(timestamp);";
    const char* idx2 = "CREATE INDEX IF NOT EXISTS idx_msg_sender ON messages(sender_id);";
    const char* idx3 = "CREATE INDEX IF NOT EXISTS idx_msg_receiver ON messages(receiver_id);";

    char* err = nullptr;
    auto exec = [&](const char* sql) {
        if (sqlite3_exec(m_db, sql, nullptr, nullptr, &err) != SQLITE_OK) {
            LOG_ERROR("SQL error: {}", err);
            sqlite3_free(err);
            return false;
        }
        return true;
        };
    return exec(users_tbl) && exec(msgs_tbl) && exec(idx1) && exec(idx2) && exec(idx3);
}

std::string MessengerServer::generateId() {
    return generateUUID();
}

bool MessengerServer::dbRegisterUser(const std::string& username,
    const std::string& display_name,
    const std::string& birthday,
    std::string& out_user_id,
    std::string& out_display_name,
    std::string& out_error) {
    sqlite3_stmt* stmt;
    // Check for existing username
    const char* check_sql = "SELECT id FROM users WHERE username = ?;";
    if (sqlite3_prepare_v2(m_db, check_sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            out_error = "Username already taken";
            sqlite3_finalize(stmt);
            return false;
        }
        sqlite3_finalize(stmt);
    }

    std::string id = generateId();
    std::string final_display_name = display_name.empty() ? username : display_name;

    const char* insert_sql = R"(
        INSERT INTO users (id, username, display_name, bio, avatar_path, birthday, is_online)
        VALUES (?, ?, ?, '', '', ?, 1);
    )";
    if (sqlite3_prepare_v2(m_db, insert_sql, -1, &stmt, nullptr) != SQLITE_OK) {
        out_error = "DB error";
        return false;
    }
    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, final_display_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, birthday.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        out_error = "Could not create user";
        sqlite3_finalize(stmt);
        return false;
    }
    sqlite3_finalize(stmt);
    out_user_id = id;
    out_display_name = final_display_name;
    return true;
}

bool MessengerServer::dbLoginUser(const std::string& username,
    std::string& out_user_id,
    std::string& out_display_name,
    std::string& out_error) {
    sqlite3_stmt* stmt;
    const char* sql = "SELECT id, display_name FROM users WHERE username = ?;";
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        out_error = "DB error";
        return false;
    }
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        out_user_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        out_display_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        sqlite3_finalize(stmt);

        // Set online status
        const char* update_sql = "UPDATE users SET is_online = 1 WHERE id = ?;";
        if (sqlite3_prepare_v2(m_db, update_sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, out_user_id.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
        }
        return true;
    }
    sqlite3_finalize(stmt);
    out_error = "User not found";
    return false;
}

std::string MessengerServer::dbGetUsers(const std::string& exclude_user_id) {
    nlohmann::json users_arr = nlohmann::json::array();
    const char* sql = "SELECT id, username, display_name, is_online FROM users WHERE id != ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, exclude_user_id.c_str(), -1, SQLITE_TRANSIENT);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            nlohmann::json u;
            std::string uid = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            u["id"] = uid;
            u["username"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            u["display_name"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            u["is_online"] = sqlite3_column_int(stmt, 3) != 0;

            // Attach last message preview
            appendLastMessageJson(u, exclude_user_id, uid);
            users_arr.push_back(u);
        }
        sqlite3_finalize(stmt);
    }
    return users_arr.dump();
}

bool MessengerServer::dbSendMessage(const std::string& sender_id, const std::string& receiver_id,
    const std::string& text, std::string& out_message_id,
    long long& out_timestamp) {
    out_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    out_message_id = generateId();

    const char* sql = R"(
        INSERT INTO messages (id, sender_id, receiver_id, text, timestamp, status)
        VALUES (?, ?, ?, ?, ?, 1);
    )";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_text(stmt, 1, out_message_id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, sender_id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, receiver_id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, text.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 5, out_timestamp);
    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

std::string MessengerServer::dbGetMessages(const std::string& user1, const std::string& user2,
    int limit, int offset) {
    nlohmann::json arr = nlohmann::json::array();
    const char* sql = R"(
        SELECT id, sender_id, receiver_id, text, timestamp, status
        FROM messages
        WHERE (sender_id = ? AND receiver_id = ?)
           OR (sender_id = ? AND receiver_id = ?)
        ORDER BY timestamp DESC
        LIMIT ? OFFSET ?;
    )";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) return arr.dump();
    sqlite3_bind_text(stmt, 1, user1.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, user2.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, user2.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, user1.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 5, limit);
    sqlite3_bind_int(stmt, 6, offset);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        nlohmann::json m;
        m["id"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        m["sender_id"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        m["receiver_id"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        m["text"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        m["timestamp"] = sqlite3_column_int64(stmt, 4);
        m["status"] = sqlite3_column_int(stmt, 5);
        arr.push_back(m);
    }
    sqlite3_finalize(stmt);
    return arr.dump();
}

std::string MessengerServer::dbGetChats(const std::string& user_id) {
    nlohmann::json arr = nlohmann::json::array();

    const char* sql = R"(
        SELECT DISTINCT
            CASE WHEN sender_id = ? THEN receiver_id ELSE sender_id END AS partner_id
        FROM messages
        WHERE sender_id = ? OR receiver_id = ?
        ORDER BY partner_id;
    )";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, user_id.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, user_id.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, user_id.c_str(), -1, SQLITE_TRANSIENT);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string partner_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            nlohmann::json chat;
            chat["partner_id"] = partner_id;

            // Look up partner's user info
            const char* user_sql = "SELECT username, display_name, is_online FROM users WHERE id = ?;";
            sqlite3_stmt* user_stmt;
            if (sqlite3_prepare_v2(m_db, user_sql, -1, &user_stmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_text(user_stmt, 1, partner_id.c_str(), -1, SQLITE_TRANSIENT);
                if (sqlite3_step(user_stmt) == SQLITE_ROW) {
                    chat["username"] = reinterpret_cast<const char*>(sqlite3_column_text(user_stmt, 0));
                    chat["display_name"] = reinterpret_cast<const char*>(sqlite3_column_text(user_stmt, 1));
                    chat["is_online"] = sqlite3_column_int(user_stmt, 2) != 0;
                }
                sqlite3_finalize(user_stmt);
            }

            // Attach last message preview
            appendLastMessageJson(chat, user_id, partner_id);
            arr.push_back(chat);
        }
        sqlite3_finalize(stmt);
    }
    return arr.dump();
}

std::string MessengerServer::dbSearchUsers(const std::string& exclude_user_id, const std::string& query) {
    nlohmann::json arr = nlohmann::json::array();
    const char* sql = R"(
        SELECT id, username, display_name, is_online FROM users
        WHERE id != ? AND (username LIKE ? OR display_name LIKE ?)
        ORDER BY username;
    )";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        std::string like_query = "%" + query + "%";
        sqlite3_bind_text(stmt, 1, exclude_user_id.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, like_query.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, like_query.c_str(), -1, SQLITE_TRANSIENT);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            nlohmann::json u;
            u["id"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            u["username"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            u["display_name"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            u["is_online"] = sqlite3_column_int(stmt, 3) != 0;
            arr.push_back(u);
        }
        sqlite3_finalize(stmt);
    }
    return arr.dump();
}


// Helper to attach last message preview to a JSON object
void MessengerServer::appendLastMessageJson(nlohmann::json& target,
    const std::string& current_user_id,
    const std::string& partner_id) {
    const char* last_msg_sql = R"(
        SELECT text, timestamp, status FROM messages
        WHERE (sender_id = ? AND receiver_id = ?)
           OR (sender_id = ? AND receiver_id = ?)
        ORDER BY timestamp DESC LIMIT 1;
    )";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, last_msg_sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, current_user_id.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, partner_id.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, partner_id.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, current_user_id.c_str(), -1, SQLITE_TRANSIENT);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            target["last_message"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            target["last_message_timestamp"] = sqlite3_column_int64(stmt, 1);
            target["last_message_status"] = sqlite3_column_int(stmt, 2);
        }
        else {
            target["last_message"] = "";
            target["last_message_timestamp"] = 0;
            target["last_message_status"] = 6;   // default
        }
        sqlite3_finalize(stmt);
    }
}