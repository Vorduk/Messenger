#include "SQLiteMessageRepository.h"
#include <cstdio>
#include <chrono>
#include "LogMacros.h"
#include "ChatListItem.h"

// Convert enum to int
static int statusToInt(MessageStatus message_status) {
    switch (message_status) {
    case MessageStatus::Sending:   return 0;
    case MessageStatus::Sent:      return 1;
    case MessageStatus::Delivered: return 2;
    case MessageStatus::Read:      return 3;
    case MessageStatus::Failed:    return 4;
    default: return 0;
    }
}

// Convert int to enum
static MessageStatus intToStatus(int i) {
    switch (i) {
    case 0: return MessageStatus::Sending;
    case 1: return MessageStatus::Sent;
    case 2: return MessageStatus::Delivered;
    case 3: return MessageStatus::Read;
    case 4: return MessageStatus::Failed;
    default: return MessageStatus::Sending;
    }
}

// Constructor
SQLiteMessageRepository::SQLiteMessageRepository(const std::string& db_path, int max_cached_messages)
    : m_db_path(db_path), m_max_cached_messages(max_cached_messages) {
    if (sqlite3_open(db_path.c_str(), &m_db) != SQLITE_OK) {
        LOG_ERROR("Cannot open database: {}", sqlite3_errmsg(m_db));
        return;
    }

    sqlite3_exec(m_db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);
    createTables();
}

// Destructor
SQLiteMessageRepository::~SQLiteMessageRepository() {
    if (m_db) {
        sqlite3_close(m_db);
    }
}


///////////////////////
// User operations
///////////////////////

bool SQLiteMessageRepository::createUser(const User& user) {
    // Query.
    const char* sql_query = R"(
        INSERT INTO users (id, username, display_name, bio, avatar_path, birthday, 
                          is_online, show_online, show_typing)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);
    )";

    // Compile sql text to byte code.
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql_query, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    // Bind params (instaead of ? symbols).
    sqlite3_bind_text(stmt, 1, user.id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, user.username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, user.display_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, user.bio.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, user.avatar_path.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, user.birthday.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 7, user.is_online ? 1 : 0);
    sqlite3_bind_int(stmt, 8, user.show_online ? 1 : 0);
    sqlite3_bind_int(stmt, 9, user.show_typing ? 1 : 0);

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE); // Execute query.

    // Free memory.
    sqlite3_finalize(stmt); 

    return ok;
}

std::optional<User> SQLiteMessageRepository::getUserById(const std::string& id) {
    // Query.
    const char* sql_query = "SELECT id, username, display_name, bio, avatar_path, birthday, "
        "is_online, show_online, show_typing FROM users WHERE id = ?;";

    // Compile sql text to byte code.
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql_query, -1, &stmt, nullptr) != SQLITE_OK)
        return std::nullopt;

    // Bind params (instaead of ? symbols).
    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);

    // Execute query.
    std::optional<User> result;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        result = rowToUser(stmt);
    }

    // Free memory.
    sqlite3_finalize(stmt); 

    return result;
}

std::optional<User> SQLiteMessageRepository::getUserByUsername(const std::string& username) {
    // Query.
    const char* sql_query = "SELECT id, username, display_name, bio, avatar_path, birthday, "
        "is_online, show_online, show_typing FROM users WHERE username = ?;";

    // Compile sql text to byte code.
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql_query, -1, &stmt, nullptr) != SQLITE_OK)
        return std::nullopt;

    // Bind params (instaead of ? symbols).
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);

    // Execute query.
    std::optional<User> result;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        result = rowToUser(stmt);
    }

    // Free memory.
    sqlite3_finalize(stmt);

    return result;
}

bool SQLiteMessageRepository::updateUser(const User& user) {
    // Query.
    const char* sql_query = R"(
        UPDATE users SET display_name=?, bio=?, avatar_path=?, birthday=?,
               is_online=?, show_online=?, show_typing=?
        WHERE id=?;
    )";

    // Compile sql text to byte code.
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql_query, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    // Bind params (instaead of ? symbols).
    sqlite3_bind_text(stmt, 1, user.display_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, user.bio.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, user.avatar_path.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, user.birthday.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 5, user.is_online ? 1 : 0);
    sqlite3_bind_int(stmt, 6, user.show_online ? 1 : 0);
    sqlite3_bind_int(stmt, 7, user.show_typing ? 1 : 0);
    sqlite3_bind_text(stmt, 8, user.id.c_str(), -1, SQLITE_TRANSIENT);

    // Execute query.
    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);

    // Free memory.
    sqlite3_finalize(stmt);

    return ok;
}

bool SQLiteMessageRepository::deleteUser(const std::string& id) {
    // Query.
    const char* sql_query = "DELETE FROM users WHERE id = ?;";

    // Compile sql text to byte code.
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql_query, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    // Bind params (instaead of ? symbols).
    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);

    // Execute query.
    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);

    // Free memory.
    sqlite3_finalize(stmt);

    return ok;
}

std::vector<User> SQLiteMessageRepository::getAllUsers() {
    std::vector<User> users;

    // Query.
    const char* sql_query = "SELECT id, username, display_name, bio, avatar_path, birthday, "
        "is_online, show_online, show_typing FROM users;";

    // Compile sql text to byte code.
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql_query, -1, &stmt, nullptr) != SQLITE_OK)
        return users;

    // Execute query.
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        users.push_back(rowToUser(stmt));
    }

    // Free memory.
    sqlite3_finalize(stmt);

    return users;
}

User SQLiteMessageRepository::rowToUser(sqlite3_stmt* stmt) {
    User u;
    u.id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    u.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    u.display_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));

    const char* text = nullptr;
    text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    u.bio = text ? text : "";
    text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
    u.avatar_path = text ? text : "";
    text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
    u.birthday = text ? text : "";

    u.is_online = sqlite3_column_int(stmt, 6) != 0;
    u.show_online = sqlite3_column_int(stmt, 7) != 0;
    u.show_typing = sqlite3_column_int(stmt, 8) != 0;
    return u;
}


///////////////////////
// Message operations
///////////////////////

bool SQLiteMessageRepository::saveMessage(const Message& msg) {
    const char* sql_query = R"(
        INSERT OR REPLACE INTO messages (id, sender_id, receiver_id, text, timestamp, status)
        VALUES (?, ?, ?, ?, ?, ?);
    )";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql_query, -1, &stmt, nullptr) != SQLITE_OK) {
        LOG_ERROR("saveMessage: prepare failed: {}", sqlite3_errmsg(m_db));
        return false;
    }

    sqlite3_int64 timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        msg.timestamp.time_since_epoch()).count();

    sqlite3_bind_text(stmt, 1, msg.id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, msg.sender_id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, msg.receiver_id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, msg.text.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 5, timestamp);
    sqlite3_bind_int(stmt, 6, statusToInt(msg.status));

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    if (!ok) {
        LOG_ERROR("saveMessage: step failed: {}", sqlite3_errmsg(m_db));
    }
    sqlite3_finalize(stmt);
    return ok;
}

bool SQLiteMessageRepository::updateMessageStatus(const std::string& message_id, MessageStatus status) {
    // Query.
    const char* sql_query = "UPDATE messages SET status = ? WHERE id = ?;";

    // Compile sql text to byte code.
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql_query, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    // Bind params (instaead of ? symbols).
    sqlite3_bind_int(stmt, 1, statusToInt(status));
    sqlite3_bind_text(stmt, 2, message_id.c_str(), -1, SQLITE_TRANSIENT);

    // Execute query.
    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);

    // Free memory.
    sqlite3_finalize(stmt);

    return ok;
}

std::vector<Message> SQLiteMessageRepository::getMessagesBetween(const std::string& user1_id, const std::string& user2_id, int limit, int offset) {
    std::vector<Message> messages;

    // Query.
    const char* sql_query = R"(
        SELECT id, sender_id, receiver_id, text, timestamp, status
        FROM messages
        WHERE (sender_id = ? AND receiver_id = ?)
           OR (sender_id = ? AND receiver_id = ?)
        ORDER BY timestamp DESC
        LIMIT ? OFFSET ?;
    )";

    // Compile sql text to byte code.
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql_query, -1, &stmt, nullptr) != SQLITE_OK)
        return messages;

    // Bind params (instaead of ? symbols).
    sqlite3_bind_text(stmt, 1, user1_id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, user2_id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, user2_id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, user1_id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 5, limit);
    sqlite3_bind_int(stmt, 6, offset);

    // Execute query.
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        messages.push_back(rowToMessage(stmt));
    }

    // Free memory.
    sqlite3_finalize(stmt);

    return messages;
}

std::vector<Message> SQLiteMessageRepository::getUnreadMessages(const std::string& user_id) {
    std::vector<Message> messages;

    // Query.
    const char* sql_query = R"(
        SELECT id, sender_id, receiver_id, text, timestamp, status
        FROM messages
        WHERE receiver_id = ? 
          AND (status = ? OR status = ?)
        ORDER BY timestamp DESC;
    )";

    // Compile sql text to byte code.
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql_query, -1, &stmt, nullptr) != SQLITE_OK)
        return messages;

    // Bind params (instaead of ? symbols).
    sqlite3_bind_text(stmt, 1, user_id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, statusToInt(MessageStatus::Sent));
    sqlite3_bind_int(stmt, 3, statusToInt(MessageStatus::Delivered));

    // Execute query.
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        messages.push_back(rowToMessage(stmt));
    }

    // Free memory.
    sqlite3_finalize(stmt);

    return messages;
}

Message SQLiteMessageRepository::rowToMessage(sqlite3_stmt* stmt) {
    Message m;
    m.id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    m.sender_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    m.receiver_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    m.text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    sqlite3_int64 ts = sqlite3_column_int64(stmt, 4);
    m.timestamp = std::chrono::system_clock::time_point(std::chrono::milliseconds(ts));
    m.status = intToStatus(sqlite3_column_int(stmt, 5));
    return m;
}


///////////////////////
// Cache management
///////////////////////

void SQLiteMessageRepository::setMaxCachedMessages(int max_cached_messages) {
    m_max_cached_messages = max_cached_messages;
    pruneOldMessages();
}

void SQLiteMessageRepository::pruneOldMessages() {
    // Query.
    const char* sql = R"(
        DELETE FROM messages WHERE id IN (
            SELECT id FROM messages
            ORDER BY timestamp ASC
            LIMIT -1 OFFSET ?
        );
    )";

    // Compile sql text to byte code.
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return;

    // Bind params (instaead of ? symbols).
    sqlite3_bind_int(stmt, 1, m_max_cached_messages);

    // Execute query.
    sqlite3_step(stmt);

    // Free memory.
    sqlite3_finalize(stmt);
}


///////////////////////
// Tables creation
///////////////////////

bool SQLiteMessageRepository::createTables() {

    // Queries.
    const char* users_table = R"(
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

    const char* messages_table = R"(
    CREATE TABLE IF NOT EXISTS messages (
        id TEXT PRIMARY KEY,
        sender_id TEXT NOT NULL,
        receiver_id TEXT NOT NULL,
        text TEXT NOT NULL,
        timestamp INTEGER NOT NULL,
        status INTEGER DEFAULT 0
        );
    )";

    const char* chat_cache_table = R"(
        CREATE TABLE IF NOT EXISTS chat_cache (
            user_id TEXT NOT NULL,
            partner_id TEXT NOT NULL,
            username TEXT,
            display_name TEXT,
            is_online INTEGER DEFAULT 0,
            last_message TEXT,
            last_message_timestamp INTEGER,
            last_message_status INTEGER DEFAULT 0,
            PRIMARY KEY (user_id, partner_id)
        );
    )";

    const char* index1 = "CREATE INDEX IF NOT EXISTS idx_messages_timestamp ON messages(timestamp);";
    const char* index2 = "CREATE INDEX IF NOT EXISTS idx_messages_sender ON messages(sender_id);";
    const char* index3 = "CREATE INDEX IF NOT EXISTS idx_messages_receiver ON messages(receiver_id);";

    return ExecuteSQLWithLogging(users_table) &&
        ExecuteSQLWithLogging(messages_table) &&
        ExecuteSQLWithLogging(chat_cache_table) &&
        ExecuteSQLWithLogging(index1) &&
        ExecuteSQLWithLogging(index2) &&
        ExecuteSQLWithLogging(index3);
}

bool SQLiteMessageRepository::ExecuteSQLWithLogging(const char* sql)
{
    char* error_message = nullptr;
    if (sqlite3_exec(m_db, sql, nullptr, nullptr, &error_message) != SQLITE_OK) {
        LOG_ERROR("SQL error in [{}]: {}", sql, error_message);
        sqlite3_free(error_message);
        return false;
    }
    return true;
}

bool SQLiteMessageRepository::cacheChatList(const std::string& userId, const std::vector<ChatListItem>& chats) {
    const char* delSql = "DELETE FROM chat_cache WHERE user_id = ?;";
    sqlite3_stmt* delStmt;
    if (sqlite3_prepare_v2(m_db, delSql, -1, &delStmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(delStmt, 1, userId.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_step(delStmt);
        sqlite3_finalize(delStmt);
    }

    const char* insSql = R"(
        INSERT INTO chat_cache (user_id, partner_id, username, display_name, is_online,
            last_message, last_message_timestamp, last_message_status)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?);
    )";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(m_db, insSql, -1, &stmt, nullptr) != SQLITE_OK) {
        LOG_ERROR("cacheChatList prepare failed: {}", sqlite3_errmsg(m_db));
        return false;
    }

    for (const auto& chat : chats) {
        sqlite3_bind_text(stmt, 1, userId.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, chat.user.id.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, chat.user.username.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, chat.user.display_name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 5, chat.user.is_online ? 1 : 0);
        sqlite3_bind_text(stmt, 6, chat.last_message.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 7, std::chrono::duration_cast<std::chrono::milliseconds>(chat.last_message_time.time_since_epoch()).count());
        sqlite3_bind_int(stmt, 8, static_cast<int>(chat.last_message_status));
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            // eerooer
        }
        sqlite3_reset(stmt);
    }
    sqlite3_finalize(stmt);
    return true;
}

std::vector<ChatListItem> SQLiteMessageRepository::getCachedChatList(const std::string& userId) {
    std::vector<ChatListItem> result;
    const char* sql = "SELECT partner_id, username, display_name, is_online, last_message, last_message_timestamp, last_message_status FROM chat_cache WHERE user_id = ? ORDER BY last_message_timestamp DESC;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    
        sqlite3_bind_text(stmt, 1, userId.c_str(), -1, SQLITE_TRANSIENT);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            ChatListItem item;
            item.user.id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            item.user.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            item.user.display_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            item.user.is_online = sqlite3_column_int(stmt, 3) != 0;
            item.last_message = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            sqlite3_int64 ts = sqlite3_column_int64(stmt, 5);
            item.last_message_time = ts > 0 ? std::chrono::system_clock::time_point(std::chrono::milliseconds(ts)) : std::chrono::system_clock::time_point::min();
            item.last_message_status = static_cast<MessageStatus>(sqlite3_column_int(stmt, 6));
            result.push_back(item);
        }
        sqlite3_finalize(stmt);
    }
    else {
        LOG_ERROR("getCachedChatList prepare failed: {}", sqlite3_errmsg(m_db));
        return result;      
    }
    return result;
}

bool SQLiteMessageRepository::saveMessages(const std::vector<Message>& messages) {
    for (const auto& msg : messages) {
        if (!saveMessage(msg)) return false;
    }
    return true;
}
