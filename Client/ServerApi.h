#pragma once
#include "IServerAPI.h"
#include <memory>

class AsyncNetworkClient;

class ServerAPI : public IServerAPI {
public:
    explicit ServerAPI(AsyncNetworkClient& client);
    void registerUser(const std::string& username, std::function<void(bool, const std::string&)> callback) override;
    
    void login(const std::string& username, std::function<void(bool, const std::string&)> callback) override;
    
    void getUsers(const std::string& user_id, std::function<void(std::vector<User>)> callback) override;
    
    void sendMessage(const std::string& sender_id, const std::string& receiver_id, const std::string& text, std::function<void(bool, const std::string&)> callback) override;
    
    void getMessages(const std::string& user_id, const std::string& partner_id, int limit, int offset, std::function<void(std::vector<Message>)> callback) override;

private:
    AsyncNetworkClient& m_client;
};