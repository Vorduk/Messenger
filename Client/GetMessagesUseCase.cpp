#include "pch.h"
#include "GetMessagesUseCase.h"
#include "GetMessagesUseCase.h"

GetMessagesUseCase::GetMessagesUseCase(IServerAPI& api, IMessageRepository& local_repo)
    : m_api(api), m_local_repo(local_repo) {
}

void GetMessagesUseCase::execute(const std::string& user_id, const std::string& partner_id,
    int limit, int offset, std::function<void(std::vector<Message>)> callback)
{
    m_api.getMessages(user_id, partner_id, limit, offset,
        [this, user_id, partner_id, limit, offset, callback](std::vector<Message> messages) {
            if (!messages.empty()) {
                m_local_repo.saveMessages(messages);
                callback(std::move(messages));
            }
            else {
                std::vector<Message> local_messages = m_local_repo.getMessagesBetween(user_id, partner_id, limit, offset);
                callback(std::move(local_messages));
            }
        });
}