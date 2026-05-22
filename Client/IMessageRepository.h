#pragma once
#include "Message.h"
#include "User.h"
#include <vector>
#include <optional>
#include <string>
#include "ChatListItem.h"

/**
 * @brief Interface for message persistence.
 * Implementations handle local caching (SQLite, in-memory, etc.).
 */
class IMessageRepository {
public:
    virtual ~IMessageRepository() = default;

    // User operations.
    virtual bool createUser(const User& user) = 0;
    virtual std::optional<User> getUserById(const std::string& id) = 0;
    virtual std::optional<User> getUserByUsername(const std::string& username) = 0;
    virtual bool updateUser(const User& user) = 0;
    virtual bool deleteUser(const std::string& id) = 0;
    virtual std::vector<User> getAllUsers() = 0;

    // Message operations.
    virtual bool saveMessage(const Message& msg) = 0;
    virtual bool updateMessageStatus(const std::string& message_id, MessageStatus status) = 0;
    virtual std::vector<Message> getMessagesBetween( const std::string& user1_id, const std::string& user2_id, int limit = 50, int offset = 0) = 0; // For paging download
    virtual std::vector<Message> getUnreadMessages(const std::string& user_id) = 0;
    virtual bool saveMessages(const std::vector<Message>& messages) = 0;

    // Cache management.
    virtual void setMaxCachedMessages(int max_cached_messages) = 0;
    virtual void pruneOldMessages() = 0; // Remove messages beyond the limit
    virtual bool cacheChatList(const std::string& userId, const std::vector<ChatListItem>& chats) = 0;
    virtual std::vector<ChatListItem> getCachedChatList(const std::string& userId) = 0;
};