#include "GetUsersUseCase.h"

GetUsersUseCase::GetUsersUseCase(IServerAPI& server_api) 
    : m_server_api(server_api) {
}

void GetUsersUseCase::search(const std::string& current_user_id, const std::string& query, std::function<void(bool success, std::vector<ChatListItem>)> callback) {
    m_server_api.searchUsers(current_user_id, query, std::move(callback));
}