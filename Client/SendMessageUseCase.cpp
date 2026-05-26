#include "pch.h"
#include "SendMessageUseCase.h"
#include "UUIDGenerator.h"

SendMessageUseCase::SendMessageUseCase(IServerAPI& server, ISendMessagePresenter& presenter, IMessageRepository& local_repo)
    : m_server(server), m_presenter(presenter), m_local_repo(local_repo) {
}

void SendMessageUseCase::execute(const std::string& sender_id, const std::string& receiver_id,
    const std::string& text) {
    if (text.empty()) {
        m_presenter.onError("Message cannot be empty");
        return;
    }

    Message localMsg(sender_id, receiver_id, text);
    localMsg.id = generateUUID();
    localMsg.status = MessageStatus::Sending;
    m_local_repo.saveMessage(localMsg);

    std::string local_id = localMsg.id;
    m_server.sendMessage(sender_id, receiver_id, text,
        [this, local_id, text](bool success, const std::string& msg_id_or_error) {
            if (success) {
                m_local_repo.updateMessageStatus(local_id, MessageStatus::Sent);
                m_presenter.onMessageSent(local_id, text, "Message sent");
            }
            else {
                m_local_repo.updateMessageStatus(local_id, MessageStatus::Failed);
                m_presenter.onError(msg_id_or_error);
            }
        });
}