#include "pch.h"
#include "GetChatsUseCase.h"
#include "GetChatsUseCase.h"

GetChatsUseCase::GetChatsUseCase(IServerAPI& server_api)
    : m_server_api(server_api) {
}

void GetChatsUseCase::execute(const std::string& current_user_id, std::function<void(bool, std::vector<ChatListItem>)> callback) {
    m_server_api.getChats(current_user_id, std::move(callback));
}