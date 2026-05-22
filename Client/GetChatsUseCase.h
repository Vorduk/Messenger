#pragma once
#include "IServerAPI.h"
#include "ChatListItem.h"
#include <functional>
#include <vector>

class GetChatsUseCase {
public:
    explicit GetChatsUseCase(IServerAPI& api) : m_api(api) {}
    void execute(const std::string& userId, std::function<void(bool, std::vector<ChatListItem>)> callback);
private:
    IServerAPI& m_api;
};