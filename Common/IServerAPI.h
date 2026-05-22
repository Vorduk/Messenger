#pragma once
#include <functional>
#include <string>
#include <vector>
#include "ChatListItem.h"
#include "User.h"
#include "Message.h"

class IServerAPI {
public:
    virtual ~IServerAPI() = default;

    virtual void registerUser(const std::string& username, std::function<void(bool success, const std::string& user_id_or_error)> callback) = 0;

    virtual void login(const std::string& username, std::function<void(bool success, const std::string& user_id_or_error)> callback) = 0;

    virtual void getUsers(const std::string& userId, std::function<void(std::vector<ChatListItem>)> callback) = 0;

    virtual void sendMessage(const std::string& sender_id, const std::string& receiver_id, const std::string& text, std::function<void(bool success, const std::string& msg_id_or_error)> callback) = 0;
    
    virtual void getMessages(const std::string& user_id, const std::string& partner_id, int limit, int offset, std::function<void(std::vector<Message>)> callback) = 0;

    virtual void getChats(const std::string& userId, std::function<void(bool success, std::vector<ChatListItem>)> callback) = 0;

    virtual void searchUsers(const std::string& userId, const std::string& query, std::function<void(std::vector<ChatListItem>)> callback) = 0;
};