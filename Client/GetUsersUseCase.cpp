#include "GetUsersUseCase.h"

GetUsersUseCase::GetUsersUseCase(IServerAPI& api) : m_api(api) {}

void GetUsersUseCase::search(const std::string& current_user_id, const std::string& query, std::function<void(std::vector<ChatListItem>)> callback) {
    m_api.searchUsers(current_user_id, query, std::move(callback));
}