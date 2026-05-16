#pragma once
#include "IMessageRepository.h"
#include <sqlite3.h>
#include <string>
#include <memory>

class SQLiteMessageRepository : public IMessageRepository {
public:

    /**
     * @brief Constructor
     * @param db_path Path to the SQLite database file.
     * @param max_cached Maximum number of cached messages per conversation (default 1000).
     */
    explicit SQLiteMessageRepository(const std::string& db_path, int max_cached_messages = 1000);
    ~SQLiteMessageRepository() override;

    // User operations.
    bool createUser(const User& user) override;
    std::optional<User> getUserById(const std::string& id) override;
    std::optional<User> getUserByUsername(const std::string& username) override;
    bool updateUser(const User& user) override;
    bool deleteUser(const std::string& id) override;
    std::vector<User> getAllUsers() override;

    // Message operations.
    bool saveMessage(const Message& msg) override;
    bool updateMessageStatus(const std::string& message_id, MessageStatus status) override;
    std::vector<Message> getMessagesBetween(const std::string& user1_id, const std::string& user2_id, int limit = 50, int offset = 0) override; // For paging download
    std::vector<Message> getUnreadMessages(const std::string& user_id) override;

    // Cache management.
    void setMaxCachedMessages(int max_cached_messages) override;
    void pruneOldMessages() override;

private:
    bool createTables();
    bool ExecuteSQLWithLogging(const char* sql);
    User rowToUser(sqlite3_stmt* stmt);
    Message rowToMessage(sqlite3_stmt* stmt);

    sqlite3* m_db = nullptr;
    int m_max_cached_messages;
    std::string m_db_path;

};
