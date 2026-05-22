#pragma once
#include "IServerAPI.h"
#include "ChatListItem.h"
#include <functional>
#include <vector>

class GetUsersUseCase {
public:
    explicit GetUsersUseCase(IServerAPI& api);
    void search(const std::string& current_user_id, const std::string& query, std::function<void(std::vector<ChatListItem>)> callback);
private:
    IServerAPI& m_api;
};