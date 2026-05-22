#pragma once
#include "IServerAPI.h"
#include "IMessageRepository.h"
#include "Message.h"
#include <functional>
#include <vector>

class GetMessagesUseCase {
public:
    GetMessagesUseCase(IServerAPI& api, IMessageRepository& local_repo);

    void execute(const std::string& user_id, const std::string& partner_id, int limit, int offset, std::function<void(std::vector<Message>)> callback);

private:
    IServerAPI& m_api;
    IMessageRepository& m_local_repo;
};