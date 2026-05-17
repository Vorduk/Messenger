#pragma once
#include "IServerAPI.h"
#include "Message.h"
#include <functional>
#include <vector>

class GetMessagesUseCase {
public:
    explicit GetMessagesUseCase(IServerAPI& api) : m_api(api) {}

    void execute(const std::string& user_id, const std::string& partner_id, int limit, int offset, std::function<void(std::vector<Message>)> callback) {
        m_api.getMessages(user_id, partner_id, limit, offset, callback);
    }
private:
    IServerAPI& m_api;
};