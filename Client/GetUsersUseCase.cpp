#include "GetUsersUseCase.h"

GetUsersUseCase::GetUsersUseCase(IServerAPI& api) : m_api(api) {}

void GetUsersUseCase::execute(const std::string& current_user_id, std::function<void(std::vector<ChatListItem>)> callback) {
    m_api.getUsers(current_user_id, std::move(callback));
}