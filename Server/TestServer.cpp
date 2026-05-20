#include "TestServer.h"
#include <iostream>
#include <cstring>
#include <json.hpp>
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

// ---------- Constructor / Destructor ----------

TestServer::TestServer(int port, const std::string& dbPath) {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        LOG_ERROR("WSAStartup failed");
        return;
    }
#endif

    // Open database
    if (sqlite3_open(dbPath.c_str(), &m_db) != SQLITE_OK) {
        LOG_ERROR("Cannot open database: {}", sqlite3_errmsg(m_db));
        return;
    }
    createTables();

    // Create socket
#ifdef _WIN32
    m_listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_listenFd == INVALID_SOCKET) {
        LOG_ERROR("Socket creation failed");
        return;
    }
#else
    m_listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_listenFd < 0) {
        LOG_ERROR("Socket creation failed");
        return;
    }
#endif

    int opt = 1;
    setsockopt(m_listenFd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

#ifdef _WIN32
    if (bind(m_listenFd, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
        LOG_ERROR("Bind failed");
        closesocket(m_listenFd);
        return;
    }
    if (listen(m_listenFd, SOMAXCONN) == SOCKET_ERROR) {
        LOG_ERROR("Listen failed");
        closesocket(m_listenFd);
        return;
    }
#else
    if (bind(m_listenFd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        LOG_ERROR("Bind failed");
        close(m_listenFd);
        return;
    }
    if (listen(m_listenFd, SOMAXCONN) < 0) {
        LOG_ERROR("Listen failed");
        close(m_listenFd);
        return;
    }
#endif

    LOG_INFO("Server listening on port {}", port);
}

TestServer::~TestServer() {
    stop();
#ifdef _WIN32
    closesocket(m_listenFd);
    WSACleanup();
#else
    close(m_listenFd);
#endif
    if (m_db) sqlite3_close(m_db);
}

// ---------- Run / Stop ----------

void TestServer::run() {
    m_running = true;
    while (m_running) {
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
#ifdef _WIN32
        int clientFd = accept(m_listenFd, (struct sockaddr*)&clientAddr, &clientLen);
        if (clientFd == INVALID_SOCKET) {
            if (m_running) LOG_ERROR("Accept failed");
            continue;
        }
#else
        int clientFd = accept(m_listenFd, (struct sockaddr*)&clientAddr, &clientLen);
        if (clientFd < 0) {
            if (m_running) LOG_ERROR("Accept failed");
            continue;
        }
#endif
        LOG_INFO("New client connected");
        m_threads.emplace_back(&TestServer::clientHandler, this, clientFd);
    }
}

void TestServer::stop() {
    m_running = false;
    // Join all client threads
    for (auto& t : m_threads) {
        if (t.joinable()) t.join();
    }
    m_threads.clear();
}

// ---------- Client Handler ----------

void TestServer::clientHandler(int clientFd) {
    std::string buffer;
    char tmp[4096];
    std::string currentUserId;   // запомним, если пользователь авторизовался

    while (m_running) {
        memset(tmp, 0, sizeof(tmp));
#ifdef _WIN32
        int bytes = recv(clientFd, tmp, sizeof(tmp) - 1, 0);
#else
        int bytes = recv(clientFd, tmp, sizeof(tmp) - 1, 0);
#endif
        if (bytes <= 0) {
            LOG_INFO("Client disconnected");
            break;
        }
        buffer.append(tmp, bytes);

        size_t pos;
        while ((pos = buffer.find('\n')) != std::string::npos) {
            std::string request = buffer.substr(0, pos);
            buffer.erase(0, pos + 1);

            auto [response, userId] = processRequest(request);
            response += "\n";
#ifdef _WIN32
            send(clientFd, response.c_str(), response.size(), 0);
#else
            send(clientFd, response.c_str(), response.size(), 0);
#endif

            if (!userId.empty()) {
                currentUserId = userId;
                std::lock_guard<std::mutex> lock(m_user_sockets_mutex);
                m_user_sockets[userId] = clientFd;
            }
        }
    }

    // Клиент отключился: убираем его из онлайн-списка и обновляем БД
    if (!currentUserId.empty()) {
        std::lock_guard<std::mutex> lock(m_user_sockets_mutex);
        auto it = m_user_sockets.find(currentUserId);
        if (it != m_user_sockets.end() && it->second == clientFd) {
            m_user_sockets.erase(it);
            // Сбрасываем флаг is_online в базе
            const char* sql = "UPDATE users SET is_online = 0 WHERE id = ?;";
            sqlite3_stmt* stmt;
            if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_text(stmt, 1, currentUserId.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_step(stmt);
                sqlite3_finalize(stmt);
            }
        }
    }

#ifdef _WIN32
    closesocket(clientFd);
#else
    close(clientFd);
#endif
}

// ---------- Request Processing ----------

std::pair<std::string, std::string> TestServer::processRequest(const std::string& json) {
    nlohmann::json req;
    try {
        req = nlohmann::json::parse(json);
    }
    catch (...) {
        return { R"({"status":"error","message":"Invalid JSON"})", "" };
    }

    std::string action = req.value("action", "");
    nlohmann::json resp;
    std::string authenticatedUserId;

    if (action == "register") {
        std::string username = req.value("username", "");
        std::string userId, error;
        if (registerUser(username, userId, error)) {
            resp["status"] = "ok";
            resp["user_id"] = userId;
            authenticatedUserId = userId;
        }
        else {
            resp["status"] = "error";
            resp["message"] = error;
        }
    }
    else if (action == "login") {
        std::string username = req.value("username", "");
        std::string userId, error;
        if (loginUser(username, userId, error)) {
            resp["status"] = "ok";
            resp["user_id"] = userId;
            authenticatedUserId = userId;
        }
        else {
            resp["status"] = "error";
            resp["message"] = error;
        }
    }
    else if (action == "get_users") {
        std::string userId = req.value("user_id", "");
        resp["status"] = "ok";
        resp["users"] = nlohmann::json::parse(getUsers(userId));
    }
    else if (action == "send_message") {
        std::string sender = req.value("sender_id", "");
        std::string receiver = req.value("receiver_id", "");
        std::string text = req.value("text", "");
        std::string msgId;
        long long timestamp;
        if (sendMessage(sender, receiver, text, msgId, timestamp)) {
            resp["status"] = "ok";
            resp["message_id"] = msgId;
            resp["timestamp"] = timestamp;
        }
        else {
            resp["status"] = "error";
            resp["message"] = "Failed to send message";
        }
    }
    else if (action == "get_messages") {
        std::string userId = req.value("user_id", "");
        std::string partnerId = req.value("partner_id", "");
        int limit = req.value("limit", 50);
        int offset = req.value("offset", 0);
        resp["status"] = "ok";
        resp["messages"] = nlohmann::json::parse(getMessages(userId, partnerId, limit, offset));
    }
    else {
        resp["status"] = "error";
        resp["message"] = "Unknown action";
    }

    return { resp.dump(), authenticatedUserId };
}

// ---------- Database Operations ----------

bool TestServer::createTables() {
    const char* usersTbl = R"(
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
    const char* msgsTbl = R"(
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
    return exec(usersTbl) && exec(msgsTbl) && exec(idx1) && exec(idx2) && exec(idx3);
}

std::string TestServer::generateId() {
    return generateUUID();  // общий генератор UUID
}

bool TestServer::registerUser(const std::string& username, std::string& outUserId, std::string& outError) {
    // Проверка уникальности
    sqlite3_stmt* stmt;
    const char* checkSql = "SELECT id FROM users WHERE username = ?;";
    if (sqlite3_prepare_v2(m_db, checkSql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            outError = "Username already taken";
            sqlite3_finalize(stmt);
            return false;
        }
        sqlite3_finalize(stmt);
    }

    std::string id = generateId();
    const char* insertSql = R"(
        INSERT INTO users (id, username, display_name, bio, avatar_path, birthday, is_online)
        VALUES (?, ?, ?, '', '', '', 1);
    )";
    if (sqlite3_prepare_v2(m_db, insertSql, -1, &stmt, nullptr) != SQLITE_OK) {
        outError = "DB error";
        return false;
    }
    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, username.c_str(), -1, SQLITE_TRANSIENT); // display_name = username
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        outError = "Could not create user";
        sqlite3_finalize(stmt);
        return false;
    }
    sqlite3_finalize(stmt);
    outUserId = id;
    return true;
}

bool TestServer::loginUser(const std::string& username, std::string& outUserId, std::string& outError) {
    sqlite3_stmt* stmt;
    const char* sql = "SELECT id FROM users WHERE username = ?;";
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        outError = "DB error";
        return false;
    }
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        outUserId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        sqlite3_finalize(stmt);
        // Установить онлайн
        const char* updateSql = "UPDATE users SET is_online = 1 WHERE id = ?;";
        if (sqlite3_prepare_v2(m_db, updateSql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, outUserId.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
        }
        return true;
    }
    sqlite3_finalize(stmt);
    outError = "User not found";
    return false;
}

std::string TestServer::getUsers(const std::string& excludeUserId) {
    nlohmann::json usersArr = nlohmann::json::array();
    const char* sql = "SELECT id, username, display_name, is_online FROM users WHERE id != ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, excludeUserId.c_str(), -1, SQLITE_TRANSIENT);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            nlohmann::json u;
            std::string uid = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            u["id"] = uid;
            u["username"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            u["display_name"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            u["is_online"] = sqlite3_column_int(stmt, 3) != 0;

            const char* lastMsgSql = R"(
                SELECT text, timestamp FROM messages
                WHERE (sender_id = ? AND receiver_id = ?)
                   OR (sender_id = ? AND receiver_id = ?)
                ORDER BY timestamp DESC LIMIT 1;
            )";
            sqlite3_stmt* msgStmt;
            if (sqlite3_prepare_v2(m_db, lastMsgSql, -1, &msgStmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_text(msgStmt, 1, excludeUserId.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(msgStmt, 2, uid.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(msgStmt, 3, uid.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(msgStmt, 4, excludeUserId.c_str(), -1, SQLITE_TRANSIENT);
                if (sqlite3_step(msgStmt) == SQLITE_ROW) {
                    u["last_message"] = reinterpret_cast<const char*>(sqlite3_column_text(msgStmt, 0));
                    u["last_message_timestamp"] = sqlite3_column_int64(msgStmt, 1);
                }
                else {
                    u["last_message"] = "";
                    u["last_message_timestamp"] = 0;
                }
                sqlite3_finalize(msgStmt);
            }

            usersArr.push_back(u);
        }
        sqlite3_finalize(stmt);
    }
    return usersArr.dump();
}

bool TestServer::sendMessage(const std::string& senderId, const std::string& receiverId,
    const std::string& text, std::string& outMessageId,
    long long& outTimestamp) {
    outTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    outMessageId = generateId();

    const char* sql = R"(
        INSERT INTO messages (id, sender_id, receiver_id, text, timestamp, status)
        VALUES (?, ?, ?, ?, ?, 1);
    )";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_text(stmt, 1, outMessageId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, senderId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, receiverId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, text.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 5, outTimestamp);
    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

std::string TestServer::getMessages(const std::string& user1, const std::string& user2,
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