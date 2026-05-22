#include "GetChatsUseCase.h"

void GetChatsUseCase::execute(const std::string& userId, std::function<void(bool, std::vector<ChatListItem>)> callback) {
    m_api.getChats(userId, std::move(callback));
}