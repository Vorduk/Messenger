#pragma once
#include "IServerAPI.h"
#include <memory>

class AsyncNetworkClient;

class ServerAPI : public IServerAPI {
public:
    explicit ServerAPI(AsyncNetworkClient& client);

    virtual void registerUser(const std::string& username, const std::string& display_name, const std::string& birthday, std::function<void(bool success, const std::string& user_id_or_error, const std::string& display_name)> callback) override;
    
    void login(const std::string& username, std::function<void(bool, const std::string&, const std::string&)> callback) override;
    
    void getUsers(const std::string& user_id, std::function<void(std::vector<ChatListItem>)> callback) override;
    
    void sendMessage(const std::string& sender_id, const std::string& receiver_id, const std::string& text, std::function<void(bool, const std::string&)> callback) override;
    
    void getMessages(const std::string& user_id, const std::string& partner_id, int limit, int offset, std::function<void(std::vector<Message>)> callback) override;

    void getChats(const std::string& userId, std::function<void(bool success, std::vector<ChatListItem>)> callback);

    void searchUsers(const std::string& userId, const std::string& query, std::function<void(bool success, std::vector<ChatListItem>)> callback) override;

private:
    AsyncNetworkClient& m_client;
};